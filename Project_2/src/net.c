/* net.c
 * Networking: sockets, UDP/TCP communication, upstream forwarding
 */
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../include/config.h"
#include "../include/dns.h"

/* ===== Socket Management ===== */

int setup_listen_socket(const char *addr, uint16_t port) {
    /* TODO: create UDP socket, set SO_REUSEADDR, bind to addr:port, return fd or -1 on error */
    return -1;
}

int close_socket(int fd) {
    /* TODO: close socket descriptor gracefully */
    return 0;
}

/* ===== UDP Communication ===== */

int send_response(int sock, const struct sockaddr *client_addr, socklen_t addr_len,
                  const uint8_t *packet, size_t packet_len) {
    /* TODO: sendto() packet to client_addr, return bytes sent or -1 on error */
    return 0;
}

int recv_query(int sock, uint8_t *buf, size_t buf_len,
               struct sockaddr *client_addr, socklen_t *addr_len) {
    /* TODO: recvfrom() into buf, store client_addr, return bytes received or -1 */
    return 0;
}

/* ===== Upstream Queries ===== */

int forward_to_upstream(const char *upstream_addr, uint16_t upstream_port,
                        const uint8_t *query, size_t query_len,
                        uint8_t *response, size_t response_len,
                        uint32_t timeout_ms, uint32_t retry_count) {
    /* TODO: create UDP socket to upstream, send query, wait with timeout, retry on failure, return response size or -1 */
    return -1;
}

int tcp_query_fallback(const char *upstream_addr, uint16_t upstream_port,
                       const uint8_t *query, size_t query_len,
                       uint8_t *response, size_t response_len) {
    /* TODO: TCP connect to upstream, send length-prefixed query, read length-prefixed response, return size or -1 */
    return -1;
}
