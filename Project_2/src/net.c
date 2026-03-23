/* net.c
 * Networking: sockets, UDP/TCP communication, upstream forwarding
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
typedef uint32_t u32;

#include <sys/types.h>    /* older systems need this for some types */
#include <sys/socket.h>   /* socket(), AF_INET, SOCK_DGRAM, SOL_SOCKET */
#include <netinet/in.h>   /* IPPROTO_UDP, struct sockaddr_in, INADDR_ANY */

#include "../include/config.h"
#include "../include/dns.h"

/* ===== Socket Management ===== */
u32 setup_listen_socket(const char *addr, u32 port) {
    /* TODO: create UDP socket, set SO_REUSEADDR, bind to addr:port, return fd or -1 on error */
    u32 opt = 1;
    struct sockaddr_in server_addr;
    // Creating the UDP socket.
    u32 fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    // Error handling.
    if (fd < 0) {
        perror("Error: ");
        return -1;
    }
    fprintf("Socket fd: %d\n", fd);

    // Setting up the SO_REUSEADDR option. since opt is 1, it allows rebinding to the same port quickely. 
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("reusing address error: ");
        close(fd);
        return -1;
    }

    // Initialzing the server add structure. 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(port); 
    // converts an Internet network address from its standard text presentation
    if (inet_pton(AF_INET, addr, &server_addr.sin_addr) <= 0) {
        perror("inet_pton error: ");
        close(fd);
        return -1;
    }
    
    // Bind the socket. 
    if (bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { 
        perror("Bind error: ");
        close(fd);
        return -1;
    }
    return fd;
}

u32 close_socket(u32 fd) {
    /* TODO: close socket descriptor gracefully */
    return 0;
}

/* ===== UDP Communication ===== */
u32 send_response(u32 sock, const struct sockaddr *client_addr, socklen_t addr_len,
                  const u32 *packet, size_t packet_len) {
    /* TODO: sendto() packet to client_addr, return bytes sent or -1 on error */
    return 0;
}

u32 recv_query(u32 sock, u32 *buf, size_t buf_len,
               struct sockaddr *client_addr, socklen_t *addr_len) {
    /* TODO: recvfrom() u32o buf, store client_addr, return bytes received or -1 */
    return 0;
}

/* ===== Upstream Queries ===== */

u32 forward_to_upstream(const char *upstream_addr, u32 upstream_port,
                        const u32 *query, size_t query_len,
                        u32 *response, size_t response_len,
                        u32 timeout_ms, u32 retry_count) {
    /* TODO: create UDP socket to upstream, send query, wait with timeout, retry on failure, return response size or -1 */
    return -1;
}

u32 tcp_query_fallback(const char *upstream_addr, u32 upstream_port,
                       const u32 *query, size_t query_len,
                       u32 *response, size_t response_len) {
    /* TODO: TCP connect to upstream, send length-prefixed query, read length-prefixed response, return size or -1 */
    return -1;
}
