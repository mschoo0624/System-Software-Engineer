#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../include/parser.h"

int main() {
    // Mock DNS Packet: Header (12 bytes) + Question (www.google.com)
    // "google" is at offset 16. We will point to it later.
    uint8_t packet[] = {
        0x12, 0x34, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Header
        0x03, 'w', 'w', 'w',             // [12] "www"
        0x06, 'g', 'o', 'o', 'g', 'l', 'e', // [16] "google"
        0x03, 'c', 'o', 'm',             // [23] "com"
        0x00,                            // [27] null
        0x00, 0x01, 0x00, 0x01,          // [28] Type A, Class IN
        // COMPRESSED NAME: "mail" + pointer to "google.com" (offset 16)
        0x04, 'm', 'a', 'i', 'l',        // [32] "mail"
        0xc0, 0x10                       // [37] Pointer to offset 16
    };

    struct dns_header hdr;
    char name_out[256];
    size_t consumed = 0;

    printf("--- Testing DNS Header ---\n");
    if (parse_dns_header(packet, sizeof(packet), &hdr) == 0) {
        printf("Header ID: 0x%04X, Questions: %u\n", hdr.id, hdr.qdcount);
    }

    printf("\n--- Testing Name Decoding (Normal) ---\n");
    decode_name(packet, sizeof(packet), 12, name_out, sizeof(name_out), &consumed);
    printf("Full Name: %s (Consumed: %zu bytes)\n", name_out, consumed);

    printf("\n--- Testing Name Decoding (Compressed) ---\n");
    decode_name(packet, sizeof(packet), 32, name_out, sizeof(name_out), &consumed);
    printf("Compressed Name: %s (Consumed: %zu bytes)\n", name_out, consumed);

    return 0;
}