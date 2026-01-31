Project_2 design notes

- Overview: simple UDP caching DNS resolver in C.
- Module responsibilities and interfaces described in source/header skeletons.
- Parsing: follow RFC 1035; use safe buffer readers and explicit ntoh conversions.
- Cache: LRU with TTL tracking; store full RR sets for answers.
- Networking: start with select()-based loop; later optimize with epoll.
