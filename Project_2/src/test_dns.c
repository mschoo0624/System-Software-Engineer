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
    // PACKET 1: Corrected Google packet
    // Offsets: 
    // 12: www.google.com
    // 32: mail.google.com (points to 'google.com' at 16)
    // 39: api.mail.google.com (points to 'mail' at 32)
    // 44: Answer section starting with pointer to www (0xc0 0x0c)
    uint8_t google_packet[] = {
        0x12, 0x34, 0x81, 0x80, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x03, 'w', 'w', 'w', 0x06, 'g', 'o', 'o', 'g', 'l', 'e', 0x03, 'c', 'o', 'm', 0x00,
        0x00, 0x01, 0x00, 0x01, // End of Question at offset 32
        0x04, 'm', 'a', 'i', 'l', 0xc0, 0x10,           // Offset 32 (mail. + pointer to google.com)
        0x03, 'a', 'p', 'i', 0xc0, 0x20,                 // Offset 39 (api. + pointer to mail)
        0xc0, 0x0c,                                     // Offset 44: Pointer to www.google.com
        0x00, 0x01, 0x00, 0x01,                         // Type A, Class IN
        0x00, 0x00, 0x01, 0x2c,                         // TTL 300
        0x00, 0x04,                                     // RDLength 4
        0x08, 0x08, 0x08, 0x08                          // IP 8.8.8.8
    };

    // PACKET 2: Wikipedia CDN packet
    // Offset 12: en.wikipedia.org
    // Offset 34: Answer 1 (Pointer to en.wikipedia.org)
    // Offset 64: Answer 2 (Pointer to the CNAME target at 48)
    uint8_t wiki_packet[] = {
        0x55, 0x55, 0x81, 0x80, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
        0x02, 'e', 'n', 0x09, 'w', 'i', 'k', 'i', 'p', 'e', 'd', 'i', 'a', 0x03, 'o', 'r', 'g', 0x00,
        0x00, 0x01, 0x00, 0x01,                         // Question ends at 34
        0xc0, 0x0c, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x0e, 0x10, 0x00, 0x12, // Ans 1 (Type CNAME)
        0x09, 'w', 'i', 'k', 'i', 'p', 'e', 'd', 'i', 'a', 0x03, 'm', 'a', 'p', 0x04, 't', 'e', 's', 't', 0x00,
        0xc0, 0x2e, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x04, // Ans 2 (Type A)
        198, 35, 26, 96                                // Correct Wikipedia IP
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
    offset = 44; // Start of Answer section
    if (parse_resource_record(google_packet, sizeof(google_packet), &offset, &rr) == 0) {
        printf("RR: %s, Type: %u, TTL: %u, IP: %u.%u.%u.%u\n", 
                rr.name, rr.type, rr.ttl, rr.rdata.data[0], rr.rdata.data[1], rr.rdata.data[2], rr.rdata.data[3]);
        free(rr.rdata.data);
    }

    // --- TEST 5: WIKIPEDIA FULL PARSE ---
    run_test_header("Test 5: Multi-Domain CDN (Wikipedia)");
    offset = 12; // Start after header
    struct dns_question q;
    
    if (parse_question_section(wiki_packet, sizeof(wiki_packet), &offset, 1, &q) == 0) {
        printf("Query: %s (Offset moved to %zu)\n", q.qname, offset);
    }

    // Ans 1
    if (parse_resource_record(wiki_packet, sizeof(wiki_packet), &offset, &rr) == 0) {
        printf("[Ans 1] %s, Type: %u, TTL: %u\n", rr.name, rr.type, rr.ttl);
        free(rr.rdata.data);
    }

    // Ans 2
    if (parse_resource_record(wiki_packet, sizeof(wiki_packet), &offset, &rr) == 0) {
        printf("[Ans 2] %s, Type: %u, IP: %u.%u.%u.%u\n", 
                rr.name, rr.type, rr.rdata.data[0], rr.rdata.data[1], rr.rdata.data[2], rr.rdata.data[3]);
        free(rr.rdata.data);
    }

    return 0;
}