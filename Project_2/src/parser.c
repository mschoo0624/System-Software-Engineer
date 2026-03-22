/* parser.c
 * DNS packet parsing implementation
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"

/* ===== Header Parsing ===== */
u32 parse_dns_header(const uint8_t *buf, size_t len, struct dns_header *hdr) {
    if (len < DNS_HEADER_SIZE) return -1;

    /*
    DNS header is exactly 12 bytes long.

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

    if (read_u16(buf, len, &offset, &hdr->id)      < 0) return -1;
    if (read_u16(buf, len, &offset, &hdr->flags)   < 0) return -1;
    if (read_u16(buf, len, &offset, &hdr->qdcount) < 0) return -1;
    if (read_u16(buf, len, &offset, &hdr->ancount) < 0) return -1;
    if (read_u16(buf, len, &offset, &hdr->nscount) < 0) return -1;
    if (read_u16(buf, len, &offset, &hdr->arcount) < 0) return -1;

    if (hdr->qdcount > DNS_MAX_QUESTIONS  ||
        hdr->ancount > DNS_MAX_ANSWERS    ||
        hdr->nscount > DNS_MAX_AUTHORITIES||
        hdr->arcount > DNS_MAX_ADDITIONAL) {
        return -1;
    }

    return 0;
}

/* ===== Name Decoding with Compression ===== */
u32 decode_name(const uint8_t *buf, size_t buf_len, size_t offset,
                char *out, size_t out_len, size_t *consumed) {
    /*
    Wire format: each label is prefixed by its length byte, terminated by 0x00.
      www.example.com → 03 'w''w''w' 07 'e'...'e' 03 'c''o''m' 00

    Compression pointers: two bytes with top bits 11xxxxxx xxxxxxxx
      The lower 14 bits give the absolute offset to jump to in the packet.
      Once we jump, consumed is already set — we never update it again.
    */
    if (!buf || !out || !consumed) return -1;

    size_t start = offset;   /* snapshot — consumed = pos - start when done */
    size_t pos   = offset;
    size_t out_pos = 0;
    u32    jumped = 0;
    size_t jump_count = 0;

    *consumed = 0;

    while (1) {
        uint8_t len_byte;
        if (read_u8(buf, buf_len, &pos, &len_byte) < 0) return -1;

        /* --- null terminator: end of name --- */
        if (len_byte == 0) {
            if (!jumped) {
                *consumed = pos - start;
            }
            break;
        }

        /* --- compression pointer: top two bits are 11 --- */
        if ((len_byte & 0xC0) == 0xC0) {
            uint8_t next_byte;
            if (read_u8(buf, buf_len, &pos, &next_byte) < 0) return -1;

            if (!jumped) {
                /* consumed = bytes from original start up to and including
                   the two pointer bytes.  pos is now past both of them. */
                *consumed = pos - start;
            }

            uint16_t ptr = ((uint16_t)(len_byte & 0x3F) << 8) | next_byte;

            /* Basic loop / self-reference guard */
            if (ptr >= buf_len) return -1;
            if (++jump_count > 16) return -1;

            pos    = ptr;
            jumped = 1;
            continue;
        }

        /* --- normal label: top two bits are 00 --- */
        if ((len_byte & 0xC0) == 0x00) {
            size_t label_len = len_byte;
            if (label_len == 0 || label_len > DNS_MAX_LABEL_LEN) return -1;

            /* Bounds: we need label_len more bytes starting at pos */
            if (pos + label_len > buf_len) return -1;

            /* Dot separator between labels */
            if (out_pos > 0) {
                if (out_pos >= out_len - 1) return -1;
                out[out_pos++] = '.';
            }

            /* Copy label characters */
            for (size_t i = 0; i < label_len; i++) {
                if (out_pos >= out_len - 1) return -1;
                out[out_pos++] = (char)buf[pos + i];
            }
            pos += label_len;
        } else {
            return -1;
        }
    }

    out[out_pos] = '\0';
    return 0;
}

/* ===== Question Section Parsing ===== */
u32 parse_question_section(const uint8_t *buf, size_t buf_len, size_t *offset,
                           uint16_t qdcount, struct dns_question *questions) {
    /*
    | Field  | Size     | Description                                |
    | ------ | -------- | ------------------------------------------ |
    | QNAME  | variable | Domain name, length-prefixed or compressed |
    | QTYPE  | 2 bytes  | Type of query (A, AAAA, MX, etc.)          |
    | QCLASS | 2 bytes  | Class (usually IN = 1)                     |
    */
    size_t pos = *offset;

    for (uint16_t i = 0; i < qdcount; i++) {
        size_t name_consumed = 0;

        if (decode_name(buf, buf_len, pos,
                        questions[i].qname, DNS_MAX_NAME_LEN,
                        &name_consumed) < 0) return -1;

        questions[i].qname[sizeof(questions[i].qname) - 1] = '\0';
        pos += name_consumed;

        /* FIX 5: read_u16 guards bounds internally; no extra check needed */
        if (read_u16(buf, buf_len, &pos, &questions[i].qtype)  < 0) return -1;
        if (read_u16(buf, buf_len, &pos, &questions[i].qclass) < 0) return -1;
    }

    *offset = pos;
    return 0;
}

/* ===== Resource Record Parsing ===== */
u32 parse_resource_record(const uint8_t *buf, size_t buf_len, size_t *offset,
                          struct dns_resource_record *rr) {
    /*
    NAME:     Domain name (variable, may be compressed)
    TYPE:     2 bytes  — A, AAAA, CNAME, MX, …
    CLASS:    2 bytes  — usually IN (1)
    TTL:      4 bytes  — seconds to cache
    RDLENGTH: 2 bytes  — byte count of RDATA
    RDATA:    variable — type-specific payload
    */

    size_t pos = *offset;
    size_t name_consumed = 0;

    if (decode_name(buf, buf_len, pos, rr->name, sizeof(rr->name),
                    &name_consumed) < 0) return -1;
    rr->name[sizeof(rr->name) - 1] = '\0';
    pos += name_consumed;

    if (read_u16(buf, buf_len, &pos, &rr->type)     < 0) return -1;
    if (read_u16(buf, buf_len, &pos, &rr->class)    < 0) return -1;
    if (read_u32(buf, buf_len, &pos, &rr->ttl)      < 0) return -1;
    if (read_u16(buf, buf_len, &pos, &rr->rdata.len)< 0) return -1;

    uint16_t rdlength   = rr->rdata.len;
    size_t   rdata_start = pos;
    
    if (pos + rdlength > buf_len) return -1;

    rr->rdata.data = NULL;

    if (rr->type == QTYPE_CNAME) {
        char     cname_buf[DNS_MAX_NAME_LEN];
        size_t   cname_consumed = 0;

        /* FIX 7: propagate decode_name failure */
        if (decode_name(buf, buf_len, pos, cname_buf, sizeof(cname_buf),
                        &cname_consumed) < 0) return -1;

        /* FIX 8: check strdup */
        rr->rdata.data = (uint8_t *)strdup(cname_buf);
        if (!rr->rdata.data) return -1;

    } else {
        rr->rdata.data = malloc(rdlength);
        if (!rr->rdata.data) return -1;
        memcpy(rr->rdata.data, buf + pos, rdlength);
    }

    /* Always advance by the wire RDLENGTH so the caller lands correctly
       on the next record, regardless of how we parsed the RDATA. */
    *offset = rdata_start + rdlength;
    return 0;
}