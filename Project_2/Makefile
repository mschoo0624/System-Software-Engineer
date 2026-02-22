# Minimal Makefile scaffold for Project_2
CC = gcc
CFLAGS_DEBUG = -g -O0 -std=gnu11 -Wall -Wextra -Iinclude
CFLAGS_RELEASE = -O2 -std=gnu11 -Wall -Wextra -Iinclude
SRCDIR = src
BINDIR = bin
TARGET = $(BINDIR)/resolver
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:.c=.o)

# Target for building the entire project
$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Target for debugging parser.c, util.c, and test_dns.c (excluding main.c)
debug_parser: src/parser.c src/util.c
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS_DEBUG) -o $(BINDIR)/test_parser src/parser.c src/util.c -Iinclude

.PHONY: all debug release clean format test valgrind run debug_parser

# Default build is release
all: release

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(TARGET)

release: CFLAGS = $(CFLAGS_RELEASE)
release: $(TARGET)

# Clean all build artifacts
clean:
	rm -rf $(BINDIR)/*.o $(TARGET) $(BINDIR)/test_parser $(SRCDIR)/*.o

# Code formatting using clang-format
format:
	clang-format -i $(SRCDIR)/*.c include/*.h

# Running the program
run: debug
	@echo "Run with: sudo setcap 'cap_net_bind_service=+ep' $(TARGET) to bind port 53 if needed"
	$(TARGET)

# Valgrind for memory leak checking
valgrind: debug
	valgrind --leak-check=full $(TARGET)

# Placeholder for unit tests
test:
	@echo "No tests yet. Add unit tests under tests/ and implement this target."

# Debugging with parser, util, and test_dns (excluding main.c)
debug_parser:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS_DEBUG) -o $(BINDIR)/test_parser src/test_dns.c src/parser.c src/util.c -Iinclude