/* util.c
 * Utility functions: safe buffer reads, byte order, logging
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include "../include/parser.h"

/* ===== Safe Buffer Read Functions ===== */
u32 read_u8(const uint8_t *buf, size_t len, size_t *offset, uint8_t *out) {
    // Checks the bound of the offset to prevent buffer overflow.
    if (*offset + 1 > len) return -1; // if it overflow then return -1.
    // Read one byte from the buffer at the given offset.
    *out = buf[*offset];
    *offset += 1; // Move the offset forward by 1 byte after reading.
    return 0; // Return 0 if succecced. 
}

u32 read_u16(const uint8_t *buf, size_t len, size_t *offset, uint16_t *out) {
    if (*offset + 2 > len) return -1;

    uint16_t net_val; // 2 bytes for network byte order value.
    // Copied the source buffer to net_val with size of 2 bytes.
    memcpy(&net_val, buf + *offset, sizeof(uint16_t));
    *out = ntohs(net_val);
    *offset += 2; // Move the offset forward by 2 bytes after reading.
    return 0;
}

u32 read_u32(const uint8_t *buf, size_t len, size_t *offset, uint32_t *out) {
    if (*offset + 4 > len) return -1;

    uint32_t net_val;
    memcpy(&net_val, buf + *offset, sizeof(uint32_t));
    *out = ntohl(net_val);
    *offset += 4;
    return 0;
}

/* ===== DNS Buffer Write Helpers ===== */
u32 write_u8(uint8_t *buf, size_t len, size_t *offset, uint8_t value) {
    if (buf == NULL || offset == NULL || *offset + 1 > len) return -1;

    buf[*offset] = value;
    *offset += 1;
    return 0;
}

u32 write_u16(uint8_t *buf, size_t len, size_t *offset, uint16_t value) {
    if (buf == NULL || offset == NULL || *offset + 2 > len) return -1;

    uint16_t net_val = htons(value);
    memcpy(buf + *offset, &net_val, sizeof(uint16_t));
    *offset += 2;
    return 0;
}

u32 write_u32(uint8_t *buf, size_t len, size_t *offset, uint32_t value) {
    if (buf == NULL || offset == NULL || *offset + 4 > len) return -1;

    uint32_t net_val = htonl(value);
    memcpy(buf + *offset, &net_val, sizeof(uint32_t));
    *offset += 4;
    return 0;
}

u32 write_name(const char *name, uint8_t *buf, size_t len, size_t *offset) {
    if (name == NULL || buf == NULL || offset == NULL) return -1;

    const char *cursor = name;
    size_t start = *offset;

    if (*name == '\0') {
        return write_u8(buf, len, offset, 0);
    }

    while (*cursor != '\0') {
        const char *dot = strchr(cursor, '.');
        size_t label_len = dot ? (size_t)(dot - cursor) : strlen(cursor);

        if (label_len == 0 || label_len > DNS_MAX_LABEL_LEN) return -1;
        if (*offset + 1 + label_len > len) return -1;

        buf[*offset] = (uint8_t)label_len;
        (*offset)++;
        memcpy(buf + *offset, cursor, label_len);
        *offset += label_len;

        if (dot == NULL) break;
        cursor = dot + 1;
    }

    if (*offset > len) return -1;
    if (write_u8(buf, len, offset, 0) < 0) return -1;

    (void)start;
    return 0;
}

u32 write_question(uint8_t *buf, size_t len, size_t *offset,
                   const struct dns_question *question) {
    if (question == NULL) return -1;

    if (write_name(question->qname, buf, len, offset) < 0) return -1;
    if (write_u16(buf, len, offset, question->qtype) < 0) return -1;
    if (write_u16(buf, len, offset, question->qclass) < 0) return -1;

    return 0;
}

u32 write_a_record(uint8_t *buf, size_t len, size_t *offset,
                   const char *name, const uint8_t *ip_bytes, uint32_t ttl) {
    if (name == NULL || ip_bytes == NULL) return -1;

    if (write_name(name, buf, len, offset) < 0) return -1;
    if (write_u16(buf, len, offset, QTYPE_A) < 0) return -1;
    if (write_u16(buf, len, offset, QCLASS_IN) < 0) return -1;
    if (write_u32(buf, len, offset, ttl) < 0) return -1;
    if (write_u16(buf, len, offset, 4) < 0) return -1;
    if (*offset + 4 > len) return -1;

    memcpy(buf + *offset, ip_bytes, 4);
    *offset += 4;
    return 0;
}

/* ===== Logging Helper ===== */
void log_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[DEBUG] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[INFO] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[WARN] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[ERROR] ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}
