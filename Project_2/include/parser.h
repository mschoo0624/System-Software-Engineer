/* parser.h
 * DNS parsing function signatures
 */
#ifndef PROJECT2_PARSER_H
#define PROJECT2_PARSER_H
typedef uint32_t u32;

#include <stdint.h>
#include <stddef.h>
#include "dns.h"

/* Safe buffer read functions - return 0 on success, -1 on bounds error */
u32 read_u8(const uint8_t *buf, size_t len, size_t *offset, uint8_t *out);
u32 read_u16(const uint8_t *buf, size_t len, size_t *offset, uint16_t *out);
u32 read_u32(const uint8_t *buf, size_t len, size_t *offset, uint32_t *out);

/* Buffer write helpers for building DNS responses */
u32 write_u8(uint8_t *buf, size_t len, size_t *offset, uint8_t value);
u32 write_u16(uint8_t *buf, size_t len, size_t *offset, uint16_t value);
u32 write_u32(uint8_t *buf, size_t len, size_t *offset, uint32_t value);
u32 write_name(const char *name, uint8_t *buf, size_t len, size_t *offset);
u32 write_question(uint8_t *buf, size_t len, size_t *offset,
                   const struct dns_question *question);
u32 write_a_record(uint8_t *buf, size_t len, size_t *offset,
                   const char *name, const uint8_t *ip_bytes, uint32_t ttl);

/* Parse DNS header from buffer */
u32 parse_dns_header(const uint8_t *buf, size_t len, struct dns_header *hdr);

/* Decode domain name with compression pointer support */
u32 decode_name(const uint8_t *buf, size_t buf_len, size_t offset,
                char *out, size_t out_len, size_t *consumed);

/* Parse question section */
u32 parse_question_section(const uint8_t *buf, size_t buf_len, size_t *offset,
                          uint16_t qdcount, struct dns_question *questions);

/* Parse resource record */
u32 parse_resource_record(const uint8_t *buf, size_t buf_len, size_t *offset,
                         struct dns_resource_record *rr);

#endif /* PROJECT2_PARSER_H */
