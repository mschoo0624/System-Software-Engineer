/* dns.h
 * High-level DNS types and shared definitions
 */
#ifndef PROJECT2_DNS_H
#define PROJECT2_DNS_H

#include <stdint.h>
#include <time.h>

/* ===== DNS Protocol Constants ===== */
#define DNS_HEADER_SIZE 12
#define DNS_MAX_NAME_LEN 255
#define DNS_MAX_PACKET_LEN 512
#define DNS_EDNS_MAX_LEN 4096
#define DNS_MAX_LABEL_LEN 63

/* Maximum sizes for various counts */
#define DNS_MAX_QUESTIONS    10
#define DNS_MAX_ANSWERS      200
#define DNS_MAX_AUTHORITIES  100
#define DNS_MAX_ADDITIONAL   100


/* DNS Question Types (QTYPE) */
#define QTYPE_A 1
#define QTYPE_NS 2
#define QTYPE_CNAME 5
#define QTYPE_SOA 6
#define QTYPE_MX 15
#define QTYPE_TXT 16
#define QTYPE_AAAA 28

/* DNS Class (usually IN = 1) */
#define QCLASS_IN 1

/* DNS Header Flags */
#define FLAG_QR (1 << 15)       /* Query(0) or Response(1) */
#define FLAG_AA (1 << 10)       /* Authoritative Answer */
#define FLAG_TC (1 << 9)        /* Truncated */
#define FLAG_RD (1 << 8)        /* Recursion Desired */
#define FLAG_RA (1 << 7)        /* Recursion Available */
#define RCODE_MASK 0x0F         /* Response Code */

/* ===== Structures ===== */

struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

struct dns_question {
    char qname[DNS_MAX_NAME_LEN];       /* Domain name */
    uint16_t qtype;                     /* Question type */
    uint16_t qclass;                    /* Question class (usually IN=1) */
};

struct dns_rdata {
    uint16_t type;                      /* Resource record type */
    uint8_t *data;                      /* Raw RDATA pointer */
    uint16_t len;                       /* RDATA length */
};

struct dns_resource_record {
    char name[DNS_MAX_NAME_LEN];        /* Resource name */
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    struct dns_rdata rdata;
};

#endif /* PROJECT2_DNS_H */
