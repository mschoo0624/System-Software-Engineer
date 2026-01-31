/* config.h
 * Configuration and CLI parsing
 */
#ifndef PROJECT2_CONFIG_H
#define PROJECT2_CONFIG_H

#include <stdint.h>

#define MAX_UPSTREAMS 5
#define MAX_LISTEN_ADDR 100

typedef struct {
    char listen_addr[MAX_LISTEN_ADDR];
    uint16_t listen_port;
    char upstream_servers[MAX_UPSTREAMS][MAX_LISTEN_ADDR];
    uint16_t upstream_ports[MAX_UPSTREAMS];
    int num_upstreams;
    uint32_t cache_max_entries;
    uint32_t query_timeout_ms;
    uint32_t retry_count;
    uint32_t edns_buffer_size;
    int log_level;
} config_t;

/* Parse CLI arguments and/or config file */
int config_load(int argc, char *argv[], config_t *cfg);

/* Print config for debugging */
void config_print(const config_t *cfg);

#endif /* PROJECT2_CONFIG_H */
