/* cache.h
 * DNS cache API
 */
#ifndef PROJECT2_CACHE_H
#define PROJECT2_CACHE_H

#include <stdint.h>
#include <time.h>
#include "dns.h"

/* Opaque cache type */
typedef struct dns_cache dns_cache_t;

/* Create and destroy cache */
dns_cache_t* cache_create(size_t max_entries);
void cache_destroy(dns_cache_t *cache);

/* Lookup: returns RR if found and TTL valid, NULL otherwise */
struct dns_resource_record* cache_lookup(dns_cache_t *cache,
                                         const char *qname,
                                         uint16_t qtype,
                                         uint16_t qclass);

/* Insert RR into cache with current timestamp */
int cache_insert(dns_cache_t *cache,
                 const struct dns_resource_record *rr);

/* Evict expired entries (call periodically) */
void cache_evict_expired(dns_cache_t *cache);

/* Get cache stats */
size_t cache_size(dns_cache_t *cache);

#endif /* PROJECT2_CACHE_H */
