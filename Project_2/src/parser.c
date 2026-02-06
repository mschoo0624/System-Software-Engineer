/* parser.c
 * DNS packet parsing implementation
 */
#include <stdint.h>
#include <string.h>
#include "../include/parser.h"

/* ===== Header Parsing ===== */
int parse_dns_header(const uint8_t *buf, size_t len, struct dns_header *hdr) {
    /* TODO: validate len >= 12, read 6 u16 fields from bytes 0-11, convert from network byte order, validate counts */
    if (len < DNS_HEADER_SIZE) {
        return -1; // Buffer too small to contain DNS header
    }
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
    size_t offset = 0;
    
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
    /* TODO: parse length-prefixed labels, follow compression pointers (0xC0+), prevent loops, reconstruct name string */
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
    int jump_offset = 0; // Offset to jump to if we follow a pointer
    *consumed = 0; // Total bytes consumed from the original offset.

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
            uint8_t next_byte;
            uint16_t pointer_offset;
            if (read_u8(buf, buf_len, pos + 1, &next_byte) < 0) return -1; // Read next byte for pointer calculation.

            pointer_offset = ((len_byte & 0X3FF) << 8) | next_byte; // Calculate pointer offset 14bits.
            if (pointer_offset >= buf_len) return -1; // Validate pointer offset.

            if (!jumped){
                *consumed += 2;
            }

            pos = pointer_offset; // Jump to the pointer offset.
            jumped = 1; // Set jumped flag.

            jump_offset += 1;
            if (jump_offset > 16) return -1;
            continue;
        }   

    }
    return 0;  /* 0 = success, -1 = error */
}

/* ===== Question Section Parsing ===== */
int parse_question_section(const uint8_t *buf, size_t buf_len, size_t *offset,
                          uint16_t qdcount, struct dns_question *questions) {
    /* TODO: for each question, decode_name(), read QTYPE, read QCLASS, advance offset, store in questions[] */
    return 0;  /* 0 = success, -1 = error */
}

/* ===== Resource Record Parsing ===== */

int parse_resource_record(const uint8_t *buf, size_t buf_len, size_t *offset,
                         struct dns_resource_record *rr) {
    /* TODO: decode_name() for NAME, read TYPE/CLASS/TTL/RDLENGTH, validate RDLENGTH, parse RDATA by type, advance offset */
    return 0;  /* 0 = success, -1 = error */
}