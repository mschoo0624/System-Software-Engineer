/* parser.c
 * DNS packet parsing implementation
 */
#include <stdint.h>
#include <string.h>
#include "../include/parser.h"

/* ===== Header Parsing ===== */
int parse_dns_header(const uint8_t *buf, size_t len, struct dns_header *hdr) {
    /* TODO: validate len >= 12, read 6 u16 fields from bytes 0-11, convert from network byte order, validate counts */
    if (len < DNS_HEADER_SIZE) return -1; // Buffer too small to contain DNS header
    /*
    DNS headr is exactly 12 bytes long. 
    
    Offset  Size  Field
    -------------------
    0       2     ID
    2       2     Flags
    4       2     QDCOUNT
    6       2     ANCOUNT
    8       2     NSCOUNT
    10      2     ARCOUNT
    */
    size_t offset = 0; // to track the current offset in buffer.
    
    if (read_u16(buf, len, &offset, &hdr->id) < 0) return -1; // if id read fails return -1, else skip two bytes to next field.
    //offset += 2;
    if (read_u16(buf, len, &offset, &hdr->flags) < 0) return -1;
    //offset += 2;
    if (read_u16(buf, len, &offset, &hdr->qdcount) < 0) return -1;
    //offset += 2;
    if (read_u16(buf, len, &offset, &hdr->ancount) < 0) return -1;
    //offset += 2;
    if (read_u16(buf, len, &offset, &hdr->nscount) < 0) return -1;
    //offset += 2;
    if (read_u16(buf, len, &offset, &hdr->arcount) < 0) return -1; 
    
    // Checking counts for sanity.
    if (hdr->qdcount > DNS_MAX_QUESTIONS ||
        hdr->ancount > DNS_MAX_ANSWERS ||
        hdr->nscount > DNS_MAX_AUTHORITIES ||
        hdr->arcount > DNS_MAX_ADDITIONAL) {
        return -1; // Counts exceed maximum allowed
    }
    
    return 0;  /* 0 = success, -1 = error */
}

/* ===== Name Decoding with Compression ===== */
int decode_name(const uint8_t *buf, size_t buf_len, size_t offset, char *out, size_t out_len, size_t *consumed) {
    /*
    example of encoding. 
    www.example.com
    03 w w w
    07 e x a m p l e
    03 c o m
    00 /0

    1.Read first byte → 03
    2.Read next 3 bytes → "www"
    3.Read next byte → 07
    4.Read next 7 bytes → "example"
    5.Read next byte → 03
    6.Read next 3 bytes → "com"
    7.Read next byte → 00
    8.Stop

    Length Byte = 0-63 : indicates the length of the next label.
    DNS reverses the top two bits of the length byte for the compression pointer.
    00xxxxxx → normal label
    11xxxxxx → compression pointer
    so the max length of a label is 63 bytes. "0X00111111" = 63
    */
    size_t pos = offset; // Current position in buffer.
    size_t out_pos = 0; // Current position in output.
    int jumped = 0; // Flag to indicate if we have followed a pointer.
    size_t jump_offset = 0; // Offset to jump to if we follow a pointer
    *consumed = 0; // Total bytes consumed from the original starting offset.

    if (!consumed || !out || !buf) return -1; // Validate pointers.

    while (1) {
        uint8_t len_byte; // 2 bytes for length byte.
        if (pos + 1 > buf_len) return -1; // Bounds check.

        if (read_u8(buf, buf_len, pos, &len_byte) < 0) return -1; // Read length byte.         
        // end of name check like null byte.
        if (len_byte == 0){         
            if (!jumped) {
                *consumed += 1; // Account for null byte if no jump occurred
            }
            break;
        }
        // Compressiong pointer check. 11xxxxxx → compression pointer
        if ((len_byte & 0XC0) == 0XC0) {
            uint8_t next_byte; //  Next byte of the pointer.
            uint16_t pointer_offset; // 14 bits for pointer offset.
            printf("Debugging: Compression pointer detected at position %zu\n", pos);
            printf( "Length byte: 0x%02X\n", len_byte);
            printf( "Next byte: 0x%02X\n", buf[pos + 1]);
            if (read_u8(buf, buf_len, pos + 1, &next_byte) < 0) return -1; // Read next byte for pointer calculation.

            pointer_offset = ((len_byte & 0X3F) << 8) | next_byte; // Calculate pointer offset 14bits.
            if (pointer_offset >= buf_len) return -1; // Validate pointer offset.
            if (!jumped) (*consumed) += 2; // Since compression pointer skips the jump bytes so count them. 
            
            jumped = 1; // Set jumped flag to true.
            pos = pointer_offset; // Jump to the pointer offset.
            jump_offset += 1; // Count the jump to prevent loops.

            if (jump_offset > 16) return -1;
            continue;
        }
        // Checking the reversed patterns.
        if ((len_byte & 0X3F) != 0X00) return -1; // Invalid length byte pattern

        // Normal label processing.
        if ((len_byte & 0XC0) == 0x00) {
            size_t label_len = len_byte & 0X3F; // To extract the length of the label.
            if (label_len == 0 || label_len > DNS_MAX_LABEL_LEN) return -1;
            printf("Debugging: The length of the label is %zu\n", label_len);

            if (pos + 1 + label_len > buf_len) return -1; // Bounds check.
            // Adding the "." between the labels.
            if (out_pos > 0){
                if (out_pos >= out_len - 1) return -1; // Ensure space for dot and null terminator.
                out[out_pos++] = '.'; // Add dot separator to the outputs .
            }

            // Now copying the label to output. 
            for (size_t i = 0; i < label_len; i++){
                if (out_pos >= out_len - 1) return -1; // Ensure space for null terminator.
                out[out_pos++] = buf[pos+1+i]; // Copy label character to output.
            }

            pos += 1 + label_len; // Advance position in buffer.
            if (!jumped) {
                *consumed += 1 + label_len; // Account for label length byte and label if no jump occurred
            }
        }
    }

    if (out_pos >= out_len) return -1; // Ensure space for null terminator.
    out[out_pos] = '\0'; // Null-terminate the output string.
    return 0;  /* 0 = success, -1 = error */
}

/* ===== Question Section Parsing ===== */
int parse_question_section(const uint8_t *buf, size_t buf_len, size_t *offset,
                          uint16_t qdcount, struct dns_question *questions) {
    /*
    | Field  | Size     | Description                                |
    | ------ | -------- | ------------------------------------------ |
    | QNAME  | variable | Domain name, length-prefixed or compressed |
    | QTYPE  | 2 bytes  | Type of query (A, AAAA, MX, etc.)          |
    | QCLASS | 2 bytes  | Class (usually IN = 1)                     |
    */
    size_t pos = *offset; // Current position in buffer.
    for (size_t i = 0; i< qdcount; i++) {
        size_t name_consumed = 0; // Bytes consumed for the name.
        
        // Handling the name domain QNAME.
        if (decode_name(buf, buf_len, pos, questions[i].qname, DNS_MAX_NAME_LEN, &name_consumed) < 0) return -1; // Decode domain name.
        strncpy(questions[i].qname, DNS_MAX_NAME_LEN, sizeof(questions[i].qname) - 1); // Copy decoded name to question structure.
        questions[i].qname[sizeof(questions[i].qname) - 1] = '\0'; // Ensure null termination.
        pos += name_consumed; // Advance position by consumed bytes.
        
        // Handling QTYPE. It tells the type of the records being requested.
        // if (pos + 2 > buf_len) return -1; // Bounds check.
        if (read_u16(buf, buf_len, &pos, &questions[i].qtype) < 0 || pos + 2 > buf_len) return -1; // Read QTYPE.

        // Handling QCLASS. It tells the class of the query.
        if (read_u16(buf, buf_len, &pos, &questions[i].qclass) < 0) return -1; // Read QCLASS.
    }

    return 0;  /* 0 = success, -1 = error */
}

/* ===== Resource Record Parsing ===== */
int parse_resource_record(const uint8_t *buf, size_t buf_len, size_t *offset,
                         struct dns_resource_record *rr) {
    /* TODO: decode_name() for NAME, read TYPE/CLASS/TTL/RDLENGTH, validate RDLENGTH, parse RDATA by type, advance offset */
    /*
    NAME: Domain name of the RR (can be compressed, same as QNAME)
    TYPE: What kind of record (A, AAAA, MX, CNAME, etc.) — 2 bytes
    CLASS: Usually IN (Internet) — 2 bytes
    TTL: Time-to-live in seconds — 4 bytes
    RDLENGTH: Length of RDATA — 2 bytes
    RDATA: Record-specific data — variable length depending on TYPE
    */
    size_t pos = *offset; // Current position in buffer.
    size_t name_consumed = 0; // Bytes consumed for the name.

    // Handling the NAME.
    if (decode_name(buf, buf_len, pos, rr->name, DNS_MAX_NAME_LEN, &name_consumed) < 0) return -1; // Decode domain name.
    strncpy(rr->name, DNS_MAX_NAME_LEN, sizeof(rr->name) - 1); // Copy decoded name to resource record structure.
    rr->name[sizeof((rr->name) - 1)] = '\0';
    printf("Debugging: Resource Record Name: %s, Name Consumed: %zu\n", rr->name, name_consumed);
    pos += name_consumed; // Advance position by consumed bytes.

    // Read TYPE.
    
    return 0;  /* 0 = success, -1 = error */
}