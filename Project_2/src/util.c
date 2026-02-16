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
int read_u8(const uint8_t *buf, size_t len, size_t *offset, uint8_t *out) {
    // Checks the bound of the offset to prevent buffer overflow.
    if (*offset + 1 > len) return -1; // if it overflow then return -1.
    // Read one byte from the buffer at the given offset.
    *out = buf[*offset];
    *offset += 1; // Move the offset forward by 1 byte after reading.
    return 0; // Return 0 if succecced. 
}

int read_u16(const uint8_t *buf, size_t len, size_t *offset, uint16_t *out) {
    if (*offset + 2 > len) return -1;

    uint16_t net_val; // 2 bytes for network byte order value.
    // Copied the source buffer to net_val with size of 2 bytes.
    memcpy(&net_val, buf + *offset, sizeof(uint16_t));
    *out = ntohs(net_val);
    *offset += 2; // Move the offset forward by 2 bytes after reading.
    return 0;
}

int read_u32(const uint8_t *buf, size_t len, size_t *offset, uint32_t *out) {
    if (*offset + 4 > len) return -1;

    uint32_t net_val;
    memcpy(&net_val, buf + *offset, sizeof(uint32_t));
    *out = ntohl(net_val);
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
