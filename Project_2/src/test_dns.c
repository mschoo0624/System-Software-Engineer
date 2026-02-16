#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"

int main() {
    uint8_t packet[] = {
        // [0-11] HEADER: ID=0x1234, QD=1, AN=1, NS=0, AR=0
        0x12, 0x34, 0x81, 0x80, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        // [12-27] QUESTION: www.google.com
        0x03, 'w', 'w', 'w', 0x06, 'g', 'o', 'o', 'g', 'l', 'e', 0x03, 'c', 'o', 'm', 0x00,
        0x00, 0x01, 0x00, 0x01, // Type A, Class IN
        // [32-38] TEST DATA: "mail" + Pointer to "google.com" (offset 16)
        0x04, 'm', 'a', 'i', 'l', 0xc0, 0x10,
        // [39-42] TEST DATA: "api" + Pointer to the "mail..." name above (offset 32)
        0x03, 'a', 'p', 'i', 0xc0, 0x20, 
        // [43-58] ANSWER RECORD: Name (Pointer to 12), Type A (1), Class IN (1), TTL 300, Len 4, IP 8.8.8.8
        0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x2c, 0x00, 0x04, 0x08, 0x08, 0x08, 0x08
    };

    char name_out[256];
    size_t consumed = 0;
    struct dns_resource_record rr;

    printf("--- Test 1: Normal Name ---\n");
    decode_name(packet, sizeof(packet), 12, name_out, sizeof(name_out), &consumed);
    printf("Result: %s (Consumed: %zu)\n", name_out, consumed);

    printf("\n--- Test 2: Single Level Compression ---\n");
    decode_name(packet, sizeof(packet), 32, name_out, sizeof(name_out), &consumed);
    printf("Result: %s (Consumed: %zu)\n", name_out, consumed);

    printf("\n--- Test 3: Nested Compression (Pointer to Pointer) ---\n");
    decode_name(packet, sizeof(packet), 39, name_out, sizeof(name_out), &consumed);
    printf("Result: %s (Consumed: %zu)\n", name_out, consumed);

    printf("\n--- Test 4: Resource Record Parsing ---\n");
    size_t rr_offset = 43;
    if (parse_resource_record(packet, sizeof(packet), &rr_offset, &rr) == 0) {
        printf("RR Name: %s, TTL: %u, RDLEN: %u\n", rr.name, rr.ttl, rr.rdata.len);
        if (rr.type == 1) { // Type A
            printf("IP: %u.%u.%u.%u\n", rr.rdata.data[0], rr.rdata.data[1], rr.rdata.data[2], rr.rdata.data[3]);
        }
        free(rr.rdata.data);
    }

    return 0;
}