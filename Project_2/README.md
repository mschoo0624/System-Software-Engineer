# Caching DNS Resolver (C, UDP Sockets)

## Overview
This project implements a **simple caching DNS resolver** in C using **UDP sockets**.  
The server accepts DNS queries from clients, forwards them to an upstream DNS server
(e.g., 8.8.8.8), caches responses based on TTL values, and replies to clients directly
from cache when possible.

The implementation follows **RFC 1035** and focuses on low-level networking,
binary protocol parsing, and performance optimization.


## Description
I built a lightweight DNS resolver prototype in C that uses BSD sockets to receive UDP queries, forward them to upstream servers, and send responses back to clients. The project demonstrates systems programming skills such as socket programming, protocol handling, timeouts, retries, and low-level memory/buffer management.
---

## Features
- UDP-based DNS server
- Parses raw DNS query packets
- Forwards queries to upstream DNS servers
- Caches DNS responses with TTL expiration
- Handles multiple concurrent clients using `select()`
- Supports `A` record queries
- Manual binary packet parsing (no DNS libraries)

---

## Technologies Used
- Language: **C**
- Networking: **BSD Sockets (UDP)**
- Concurrency: `select()`
- Protocol: **DNS (RFC 1035)**
- OS: Linux / macOS

---

## How It Works

1. Client sends a DNS query to the resolver
2. Resolver checks local cache:
   - If valid entry exists → respond immediately
   - Otherwise → forward request to upstream DNS server
3. Resolver parses the upstream response
4. Response is cached with TTL
5. Resolver sends response back to client

---

## Build Instructions

```bash
make
