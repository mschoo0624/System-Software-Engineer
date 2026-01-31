/* cache.c
 * In-memory DNS cache with LRU eviction and TTL expiration
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/cache.h"

struct dns_cache {
    /* TODO: define cache structure (hash table, LRU list, entry structs) */
};

dns_cache_t* cache_create(size_t max_entries) {
    /* TODO: allocate and initialize cache, set max_entries */
    return NULL;
}

void cache_destroy(dns_cache_t *cache) {
    /* TODO: free all cache entries and cache structure */
}

struct dns_resource_record* cache_lookup(dns_cache_t *cache,
                                         const char *qname,
                                         uint16_t qtype,
                                         uint16_t qclass) {
    /* TODO: hash (qname,qtype,qclass), check TTL expiration, return RR or NULL */
    return NULL;
}

int cache_insert(dns_cache_t *cache,
                 const struct dns_resource_record *rr) {
    /* TODO: hash key, store with timestamp, evict LRU on overflow */
    return 0;  /* 0 = success */
}

void cache_evict_expired(dns_cache_t *cache) {
    /* TODO: iterate entries, remove if (now - timestamp) > TTL */
}

size_t cache_size(dns_cache_t *cache) {
    /* TODO: return current number of entries */
    return 0;
}
