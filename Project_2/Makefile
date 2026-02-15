# Minimal Makefile scaffold for Project_2
CC = gcc
CFLAGS_DEBUG = -g -O0 -std=gnu11 -Wall -Wextra -Iinclude
CFLAGS_RELEASE = -O2 -std=gnu11 -Wall -Wextra -Iinclude
SRCDIR = src
BINDIR = bin
TARGET = $(BINDIR)/resolver
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:.c=.o)

.PHONY: all debug release clean format test valgrind run

all: release

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(TARGET)

release: CFLAGS = $(CFLAGS_RELEASE)
release: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BINDIR)/*.o $(TARGET)

format:
	clang-format -i $(SRCDIR)/*.c include/*.h

run: debug
	@echo "Run with: sudo setcap 'cap_net_bind_service=+ep' $(TARGET) to bind port 53 if needed"
	$(TARGET)

valgrind: debug
	valgrind --leak-check=full $(TARGET)

test:
	@echo "No tests yet. Add unit tests under tests/ and implement this target."

test_code:
	gcc src/parser.c src/util.c src/test_dns.c -I./include -o dns_test