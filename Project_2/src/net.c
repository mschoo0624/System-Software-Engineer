/* net.c
 * Networking: sockets, UDP/TCP communication, upstream forwarding
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>    /* older systems need this for some types */
#include <sys/socket.h>   /* socket(), AF_INET, SOCK_DGRAM, SOL_SOCKET */
#include <netinet/in.h>   /* IPPROTO_UDP, struct sockaddr_in, INADDR_ANY */
#include <sys/time.h>

#include "../include/config.h"
#include "../include/dns.h"

// Simple logging macro since log_warn is not defined elsewhere
#define log_warn(...) fprintf(stderr, __VA_ARGS__)

/* ===== Socket Management ===== */
int setup_listen_socket(const char *addr, int port) {
    /* TODO: create UDP socket, set SO_REUSEADDR, bind to addr:port, return fd or -1 on error */
    int opt = 1;
    struct sockaddr_in server_addr;
    // Creating the UDP socket (SOCK_DGRAM/SOCK_STREAM).
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    // Error handling.
    if (fd < 0) {
        perror("Error: ");
        return -1;
    }
    fprintf(stderr, "Socket fd: %d\n", fd); // the fd table should be starting from 3. since 0, 1 and 2 are reserved.

    // Setting up the SO_REUSEADDR option. since opt is 1, it allows rebinding to the same port quickely. 
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("reusing address error: ");
        close(fd);
        return -1;
    }

    // Initialzing the server add structure. 
    memset(&server_addr, 0, sizeof(server_addr));
    // Setting up the specified address and port. 
    server_addr.sin_family = AF_INET; // IVP4
    server_addr.sin_port = htons(port); // Sepcific port number and htons() convert to network byte order.
    // converts an Internet network address from its standard text presentation
    if (inet_pton(AF_INET, addr, &server_addr.sin_addr) <= 0) {
        perror("inet_pton error: ");
        close(fd);
        return -1;
    }
    
    // Bind the socket. (Adding the server address to the socket). 
    if (bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { 
        perror("Bind error: ");
        close(fd);
        return -1;
    }
    return fd;
}

int close_socket(int fd) {
    /* TODO: close socket descriptor gracefully */
    if (fd < 0){
        log_warn("close socket: invalid fd %d", fd);
        return -1;
    }

    if (close(fd) < 0) {
        perror("close");
        return -1;
    }
    return 0;
}

/* ===== UDP Communication ===== */
int send_response(int sock, const struct sockaddr *client_addr, socklen_t addr_len,
                  const uint8_t *packet, size_t packet_len) {
    /* TODO: sendto() packet to client_addr, return bytes sent or -1 on error */
    // handling edge cases.
    if (packet == NULL || packet_len == 0 || sock < 0) {
        log_warn("send_response: invalid args — sock=%d packet=%p len=%zu", sock, packet, packet_len);
        return -1;
    }
    // packet is the pointer to the first byte of the DNS response data in memoory, and check the packet length amount and send the answer to the client addresss. 
    ssize_t sent = sendto(sock, packet, packet_len, 0, client_addr, addr_len);
    if (sent < 0) {
        perror("error");
        return -1;
    } else if (sent != (ssize_t)packet_len) {
        log_warn("sendto: Partial send");
    }
    return (int)sent;
}

int recv_query(int sock, int *buf, size_t buf_len,
               struct sockaddr *client_addr, socklen_t *addr_len) {
    /* TODO: recvfrom() into buf, store client_addr, return bytes received or -1 */
    // Handling all the edge cases. 
    if (sock < 0 || buf == NULL || buf_len == 0 || client_addr == NULL || addr_len == NULL) {
        log_warn("Invalid");
        return -1;
    }

    // Calling the recvfrom() client_addr is where the sender's address will be stored.
    ssize_t received = recvfrom(sock, buf, buf_len, 0, client_addr, addr_len);

    if (received < 0) {
        perror("recvfrom");
        return -1;
    }
    return (int)received;
}

/* ===== Upstream Queries ===== */
int forward_to_upstream(const char *upstream_addr, int upstream_port,
                        const uint8_t *query, size_t query_len,
                        uint8_t *response, size_t response_len,
                        int timeout_ms, int retry_count) {
    /* TODO: create UDP socket to upstream, send query, wait with timeout, retry on failure, return response size or -1 */
    // handling the edge cases. 
    if (upstream_addr == NULL || query == NULL || query_len == 0 || response == NULL || response_len == 0) {
        log_warn("Invalid args");
        return -1;
    }
    
    // Created a UDP socket.
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    // Setting up the server address structure for the upstream server.
    struct sockaddr_in upstream_addr_struct;
    memset(&upstream_addr_struct, 0, sizeof(upstream_addr_struct));
    upstream_addr_struct.sin_family = AF_INET; // IPv4
    upstream_addr_struct.sin_port = htons(upstream_port); // Convert the upstream port to network byte order. 
    // Converting the adress to working network. 
    if (inet_pton(AF_INET, upstream_addr, &upstream_addr_struct.sin_addr) <= 0) {
        perror("inet_pton error: ");
        close(fd);
        return -1; 
    }
    // setting the socket timeout for receiving response from the upstream server. 
    struct timeval tv = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000
    };
        
    // Receive/Send timeout settings.
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        close(fd);
        return -1;
    }

    // Now setting up the retry to send and receive.
    retry_count = retry_count > 0 ? retry_count : 1;
    for (int attempt = 0; attempt < retry_count; attempt++){
        // Sending the data to the upstream socket address. 
        ssize_t sent = sendto(fd, query, query_len, 0,
                              (struct sockaddr *)&upstream_addr_struct,
                              sizeof(upstream_addr_struct));
        // If not sent attempting again. 
        if (sent < 0) {
            perror("sendto()");
            continue;
        }
        // Now receiving the data from data has been sent. 
        ssize_t receive = recvfrom(fd, response, response_len, 0, NULL, NULL);
        if (receive < 0) {
            perror("recvfrom()");
            continue;
        }
        close(fd);
        return (int)receive;
    }
    close(fd);
    return -1;
}

int tcp_query_fallback(const char *upstream_addr, int upstream_port,
                       const uint8_t *query, size_t query_len,
                       uint8_t *response, size_t response_len) {
    /* TODO: TCP connect to upstream, send length-prefixed query, read length-prefixed response, return size or -1 */
    return -1;
}
