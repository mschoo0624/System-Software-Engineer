#!/bin/sh
# Run the resolver in debug mode and show example dig command
set -e
./bin/resolver || true
# Example: dig @127.0.0.1 -p 5353 example.com
