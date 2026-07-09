#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int setup_listen_socket(const char *addr, int port);
int close_socket(int fd);
int send_response(int sock, const struct sockaddr *client_addr, socklen_t addr_len,
                  const uint8_t *packet, size_t packet_len);
int recv_query(int sock, int *buf, size_t buf_len,
               struct sockaddr *client_addr, socklen_t *addr_len);
int forward_to_upstream(const char *upstream_addr, int upstream_port,
                        const uint8_t *query, size_t query_len,
                        uint8_t *response, size_t response_len,
                        int timeout_ms, int retry_count);

static size_t read_user_message(const char *prompt, char *buffer, size_t buffer_size) {
    printf("%s", prompt);
    if (!fgets(buffer, (int)buffer_size, stdin)) {
        return 0;
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[--len] = '\0';
    }
    return len;
}

static void test_udp_echo_flow(void) {
    char payload[128];
    size_t payload_len = read_user_message("Enter a message for the UDP echo test: ", payload, sizeof(payload));
    if (payload_len == 0) {
        fprintf(stderr, "No message entered.\n");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        int server_sock = setup_listen_socket("127.0.0.1", 5353);
        if (server_sock < 0) {
            fprintf(stderr, "child: setup_listen_socket failed\n");
            exit(1);
        }

        int buffer[64] = {0};
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int received = recv_query(server_sock, buffer, sizeof(buffer),
                                  (struct sockaddr *)&client_addr, &addr_len);
        if (received < 0) {
            fprintf(stderr, "child: recv_query failed\n");
            close_socket(server_sock);
            exit(1);
        }

        uint8_t reply[64];
        memcpy(reply, buffer, (size_t)received);
        int sent = send_response(server_sock, (struct sockaddr *)&client_addr, addr_len,
                                  reply, (size_t)received);
        if (sent < 0) {
            fprintf(stderr, "child: send_response failed\n");
            close_socket(server_sock);
            exit(1);
        }

        printf("[child] received %d bytes and echoed them back\n", received);
        close_socket(server_sock);
        exit(0);
    }

    usleep(200000);

    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sock < 0) {
        perror("client socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5353);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (sendto(client_sock, payload, payload_len, 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("sendto");
        exit(1);
    }

    uint8_t response[128];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    ssize_t n = recvfrom(client_sock, response, sizeof(response), 0,
                         (struct sockaddr *)&from_addr, &from_len);
    if (n < 0) {
        perror("recvfrom");
        exit(1);
    }

    response[n] = '\0';
    printf("[parent] received response: %.*s\n", (int)n, (char *)response);

    int status = 0;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "child exited unexpectedly\n");
        exit(1);
    }

    close(client_sock);
}

static void test_forward_to_upstream(void) {
    char payload[128];
    size_t payload_len = read_user_message("Enter a message for the upstream forwarding test: ", payload, sizeof(payload));
    if (payload_len == 0) {
        fprintf(stderr, "No message entered.\n");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_sock < 0) {
            perror("upstream socket");
            exit(1);
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(5354);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("bind");
            close(server_sock);
            exit(1);
        }

        uint8_t buf[64];
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        ssize_t n = recvfrom(server_sock, buf, sizeof(buf), 0,
                             (struct sockaddr *)&client_addr, &len);
        if (n < 0) {
            perror("upstream recvfrom");
            close(server_sock);
            exit(1);
        }

        if (sendto(server_sock, buf, (size_t)n, 0,
                   (struct sockaddr *)&client_addr, len) < 0) {
            perror("upstream sendto");
            close(server_sock);
            exit(1);
        }

        close(server_sock);
        exit(0);
    }

    usleep(200000);

    uint8_t response[128];
    int result = forward_to_upstream("127.0.0.1", 5354, (const uint8_t *)payload, payload_len,
                                      response, sizeof(response), 500, 1);
    if (result < 0) {
        fprintf(stderr, "forward_to_upstream failed\n");
        exit(1);
    }

    printf("[forward] received %d bytes from upstream\n", result);
    printf("[forward] response: %.*s\n", result, (char *)response);

    int status = 0;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "upstream child exited unexpectedly\n");
        exit(1);
    }
}

int main(void) {
    printf("Running net.c socket tests over localhost...\n");
    test_udp_echo_flow();
    test_forward_to_upstream();
    printf("All net.c tests completed successfully.\n");
    return 0;
}
