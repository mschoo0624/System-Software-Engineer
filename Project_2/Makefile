# Minimal Makefile scaffold for Project_2
CC = gcc
CFLAGS_DEBUG = -g -O0 -std=gnu11 -Wall -Wextra -Iinclude
CFLAGS_RELEASE = -O2 -std=gnu11 -Wall -Wextra -Iinclude
SRCDIR = src
BINDIR = bin
TARGET = $(BINDIR)/resolver
SRCS = $(filter-out $(SRCDIR)/test_dns.c,$(wildcard $(SRCDIR)/*.c))
OBJS = $(SRCS:.c=.o)

# Target for building the entire project
$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

.PHONY: all debug release clean format test valgrind run debug_parser net_debug

# Default build is release
all: release

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(TARGET)

# Compile only src/net.c with debug flags
debug_net: CFLAGS = $(CFLAGS_DEBUG)
debug_net:
	$(CC) $(CFLAGS) -Iinclude -c $(SRCDIR)/net.c -o $(SRCDIR)/net.o

# Build and run the interactive net.c test harness
net_debug: CFLAGS = $(CFLAGS_DEBUG)
net_debug:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -Iinclude $(SRCDIR)/net_test.c $(SRCDIR)/net.c -o $(BINDIR)/net_test
	./$(BINDIR)/net_test

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