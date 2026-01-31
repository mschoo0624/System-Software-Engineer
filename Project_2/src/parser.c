/* parser.c
 * DNS packet parsing implementation
 */
#include <stdint.h>
#include <string.h>
#include "../include/parser.h"

/* ===== Header Parsing ===== */

int parse_dns_header(const uint8_t *buf, size_t len, struct dns_header *hdr) {
    /* TODO: validate len >= 12, read 6 u16 fields from bytes 0-11, convert from network byte order, validate counts */
    
    return 0;  /* 0 = success, -1 = error */
}

/* ===== Name Decoding with Compression ===== */

int decode_name(const uint8_t *buf, size_t buf_len, size_t offset,
                char *out, size_t out_len, size_t *consumed) {
    /* TODO: parse length-prefixed labels, follow compression pointers (0xC0+), prevent loops, reconstruct name string */
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