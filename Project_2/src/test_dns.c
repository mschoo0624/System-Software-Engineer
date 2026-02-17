#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"

void run_test_header(const char* title) {
    printf("\n==========================================\n");
    printf("   %s\n", title);
    printf("==========================================\n");
}

int main() {
    // PACKET 1: The Google / Mail / API packet (Tests 1-4)
    uint8_t google_packet[] = {
        0x12, 0x34, 0x81, 0x80, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x03, 'w', 'w', 'w', 0x06, 'g', 'o', 'o', 'g', 'l', 'e', 0x03, 'c', 'o', 'm', 0x00,
        0x00, 0x01, 0x00, 0x01,
        0x04, 'm', 'a', 'i', 'l', 0xc0, 0x10, // Offset 32
        0x03, 'a', 'p', 'i', 0xc0, 0x20,       // Offset 39
        0xc0, 0x0c, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x2c, 0x00, 0x04, // Offset 43
        0x08, 0x08, 0x08, 0x08
    };

    // PACKET 2: The Wikipedia CDN packet
    uint8_t wiki_packet[] = {
        0x55, 0x55, 0x81, 0x80, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
        0x02, 'e', 'n', 0x09, 'w', 'i', 'k', 'i', 'p', 'e', 'd', 'i', 'a', 0x03, 'o', 'r', 'g', 0x00,
        0x00, 0x01, 0x00, 0x01, // Ends at 32
        0xc0, 0x0c, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x0e, 0x10, 0x00, 0x12, // Answer 1
        0x09, 'w', 'i', 'k', 'i', 'p', 'e', 'd', 'i', 'a', 0x03, 'm', 'a', 'p', 0x04, 't', 'e', 's', 't', 0x00,
        0xc0, 0x2d, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x04, // Answer 2
        0xC6, 0x23, 0x1A, 0x60
    };

    char name_out[256];
    size_t consumed = 0;
    size_t offset = 0;
    struct dns_resource_record rr;

    // --- TESTS 1-3: NAME DECODING ---
    run_test_header("Tests 1-3: Name Decoding & Compression");
    
    decode_name(google_packet, sizeof(google_packet), 12, name_out, sizeof(name_out), &consumed);
    printf("Test 1 (Normal): %s (Consumed: %zu)\n", name_out, consumed);

    decode_name(google_packet, sizeof(google_packet), 32, name_out, sizeof(name_out), &consumed);
    printf("Test 2 (Single Compression): %s (Consumed: %zu)\n", name_out, consumed);

    decode_name(google_packet, sizeof(google_packet), 39, name_out, sizeof(name_out), &consumed);
    printf("Test 3 (Nested Compression): %s (Consumed: %zu)\n", name_out, consumed);

    // --- TEST 4: GOOGLE RESOURCE RECORD ---
    run_test_header("Test 4: Google Resource Record");
    offset = 43;
    if (parse_resource_record(google_packet, sizeof(google_packet), &offset, &rr) == 0) {
        printf("RR: %s, TTL: %u, IP: %u.%u.%u.%u\n", 
                rr.name, rr.ttl, rr.rdata.data[0], rr.rdata.data[1], rr.rdata.data[2], rr.rdata.data[3]);
        free(rr.rdata.data);
    }

    // --- TEST 5: WIKIPEDIA FULL PARSE ---
    run_test_header("Test 5: Multi-Domain CDN (Wikipedia)");
    offset = 12; // Start after header
    struct dns_question q;
    
    // Parse the Question
    if (parse_question_section(wiki_packet, sizeof(wiki_packet), &offset, 1, &q) == 0) {
        printf("Query: %s (Offset moved to %zu)\n", q.qname, offset);
    }

    // Parse Answer 1 (CNAME)
    if (parse_resource_record(wiki_packet, sizeof(wiki_packet), &offset, &rr) == 0) {
        printf("[Ans 1] %s, Type: %u, TTL: %u\n", rr.name, rr.type, rr.ttl);
        free(rr.rdata.data);
    }

    // Parse Answer 2 (A Record)
    if (parse_resource_record(wiki_packet, sizeof(wiki_packet), &offset, &rr) == 0) {
        printf("[Ans 2] %s, Type: %u, IP: %u.%u.%u.%u\n", 
                rr.name, rr.type, rr.rdata.data[0], rr.rdata.data[1], rr.rdata.data[2], rr.rdata.data[3]);
        free(rr.rdata.data);
    }

    return 0;
}