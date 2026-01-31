/*****************************************************************************************
DNS (Domian Name System) Project 2
This project implements a **simple caching DNS resolver** in C using **UDP sockets**.  
The server accepts DNS queries from clients, forwards them to an upstream DNS server
(e.g., 8.8.8.8), caches responses based on TTL values, and replies to clients directly
from cache when possible.

The implementation follows **RFC 1035** and focuses on low-level networking,
binary protocol parsing, and performance optimization.

Users -> Resolver Server -> Root Server -> (If not found in Root Server) -> Resolver Server -> TLD Server (Stroes the address information
for top level domains like .com, .org, .net) -> (If not found in TLD Server) -> Resolver Server -> Name Server.
******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>

/* Networking headers */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>

int main(void) {
    
    return 0;
}