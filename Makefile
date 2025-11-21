# JSON Parser Makefile
# Manages build, test, and benchmark processes

# ============================================================================
# Configuration
# ============================================================================

# Project info
PROJECT_NAME = jsonparser
VERSION = 0.1.0

# Compiler
CC = gcc
AR = ar

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
LIB_DIR = lib
TEST_DIR = tests
BENCH_DIR = benchmarks
DIST_DIR = dist

# Source files for library
LIB_SOURCES = $(SRC_DIR)/lexer.c \
              $(SRC_DIR)/parser.c \
              $(SRC_DIR)/json.c

LIB_HEADERS = $(INC_DIR)/lexer.h \
              $(INC_DIR)/parser.h \
              $(INC_DIR)/json.h

# Object files
LIB_OBJECTS = $(BUILD_DIR)/lexer.o \
              $(BUILD_DIR)/parser.o \
              $(BUILD_DIR)/json.o

# Library outputs
STATIC_LIB = $(LIB_DIR)/lib$(PROJECT_NAME).a
SHARED_LIB = $(LIB_DIR)/lib$(PROJECT_NAME).so

# Test files
TEST_SOURCES = $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINARIES = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_SOURCES))

# Benchmark binaries
BENCH_BINARIES = $(BENCH_DIR)/bin/bench_our_parser \
                 $(BENCH_DIR)/bin/bench_memory

# Compiler flags
CFLAGS_BASE = -Wall -Wextra -I$(INC_DIR)
CFLAGS_DEBUG = $(CFLAGS_BASE) -O0 -g -DDEBUG
CFLAGS_RELEASE = $(CFLAGS_BASE) -O3 -flto -march=native -DNDEBUG
CFLAGS_SIZE = $(CFLAGS_BASE) -Os -DNDEBUG

# Linker flags
LDFLAGS_BASE =
LDFLAGS_DEBUG = $(LDFLAGS_BASE)
LDFLAGS_RELEASE = $(LDFLAGS_BASE) -flto
LDFLAGS_SIZE = $(LDFLAGS_BASE)

# Shared library flags
SHARED_FLAGS = -shared -fPIC

# Default to release build
CFLAGS = $(CFLAGS_RELEASE)
LDFLAGS = $(LDFLAGS_RELEASE)

# Colors for output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
NC = \033[0m

# ============================================================================
# Targets
# ============================================================================

.PHONY: all clean help debug release size libs static shared test benchmark install uninstall info build-tests build-benchmarks format analyze todos check-size

# Default target
all: release

# Help
help:
	@echo "$(BLUE)JSON Parser Build System$(NC)"
	@echo ""
	@echo "$(YELLOW)Main Targets:$(NC)"
	@echo "  make              - Build release version (default)"
	@echo "  make release      - Build optimized release library"
	@echo "  make debug        - Build debug version with symbols"
	@echo "  make size         - Build size-optimized version"
	@echo "  make test         - Build and run all tests"
	@echo "  make benchmark    - Build and run benchmarks"
	@echo "  make clean        - Remove all build artifacts"
	@echo ""
	@echo "$(YELLOW)Library Targets:$(NC)"
	@echo "  make libs         - Build both static and shared libraries"
	@echo "  make static       - Build static library (.a)"
	@echo "  make shared       - Build shared library (.so)"
	@echo ""
	@echo "$(YELLOW)Installation:$(NC)"
	@echo "  make install      - Install library and headers"
	@echo "  make uninstall    - Uninstall library and headers"
	@echo ""
	@echo "$(YELLOW)Benchmarking:$(NC)"
	@echo "  make benchmark         - Run full benchmark suite (perf + memory)"
	@echo "  make benchmark-perf    - Run performance benchmarks only"
	@echo "  make benchmark-memory  - Run memory benchmarks only"
	@echo "  make benchmark-view    - Start web server & open visualization"
	@echo ""
	@echo "$(YELLOW)Analysis:$(NC)"
	@echo "  make info         - Show binary size information"
	@echo "  make check-size   - Compare sizes across build types"
	@echo ""
	@echo "$(YELLOW)Build Configurations:$(NC)"
	@echo "  Release: -O3 -flto -march=native (fastest)"
	@echo "  Debug:   -O0 -g (debugging)"
	@echo "  Size:    -Os (smallest binary)"

# ============================================================================
# Build Targets
# ============================================================================

# Release build
release: CFLAGS = $(CFLAGS_RELEASE)
release: LDFLAGS = $(LDFLAGS_RELEASE)
release: libs
	@echo "$(GREEN)✓ Release build complete$(NC)"

# Debug build
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: LDFLAGS = $(LDFLAGS_DEBUG)
debug: libs
	@echo "$(GREEN)✓ Debug build complete$(NC)"

# Size-optimized build
size: CFLAGS = $(CFLAGS_SIZE)
size: LDFLAGS = $(LDFLAGS_SIZE)
size: libs
	@echo "$(GREEN)✓ Size-optimized build complete$(NC)"

# Build both libraries
libs: static shared

# Build static library
static: $(STATIC_LIB)
	@echo "$(GREEN)✓ Static library built: $(STATIC_LIB)$(NC)"

# Build shared library
shared: $(SHARED_LIB)
	@echo "$(GREEN)✓ Shared library built: $(SHARED_LIB)$(NC)"

# ============================================================================
# Object Files
# ============================================================================

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(LIB_HEADERS) | $(BUILD_DIR)
	@echo "$(YELLOW)Compiling $<...$(NC)"
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# Library Linking
# ============================================================================

$(STATIC_LIB): $(LIB_OBJECTS) | $(LIB_DIR)
	@echo "$(YELLOW)Creating static library...$(NC)"
	$(AR) rcs $@ $^

$(SHARED_LIB): $(LIB_SOURCES) $(LIB_HEADERS) | $(LIB_DIR)
	@echo "$(YELLOW)Creating shared library...$(NC)"
	$(CC) $(CFLAGS) $(SHARED_FLAGS) $(LIB_SOURCES) -o $@

# ============================================================================
# Tests
# ============================================================================

# Build test binaries
$(BUILD_DIR)/test_%: $(TEST_DIR)/test_%.c $(STATIC_LIB)
	@echo "$(YELLOW)Building test: $@$(NC)"
	$(CC) $(CFLAGS) $< -L$(LIB_DIR) -l$(PROJECT_NAME) -o $@ $(LDFLAGS)

# Build all tests
build-tests: $(TEST_BINARIES)
	@echo "$(GREEN)✓ All tests built$(NC)"

# Run all tests
test: build-tests
	@echo "$(BLUE)=========================================$(NC)"
	@echo "$(BLUE)Running Tests$(NC)"
	@echo "$(BLUE)=========================================$(NC)"
	@for test in $(TEST_BINARIES); do \
		echo "$(YELLOW)Running $$test...$(NC)"; \
		./$$test && echo "$(GREEN)✓ $$test passed$(NC)" || echo "$(RED)✗ $$test failed$(NC)"; \
		echo ""; \
	done
	@echo "$(BLUE)=========================================$(NC)"
	@echo "$(GREEN)✓ All tests complete$(NC)"
	@echo "$(BLUE)=========================================$(NC)"

# ============================================================================
# Benchmarks
# ============================================================================

$(BENCH_DIR)/bin/bench_our_parser: $(BENCH_DIR)/src/bench_our_parser.c $(LIB_SOURCES) $(LIB_HEADERS)
	@echo "$(YELLOW)Building performance benchmark...$(NC)"
	@mkdir -p $(BENCH_DIR)/bin
	$(CC) $(CFLAGS_RELEASE) $< $(LIB_SOURCES) -I$(INC_DIR) -o $@ $(LDFLAGS_RELEASE)

$(BENCH_DIR)/bin/bench_memory: $(BENCH_DIR)/src/bench_memory.c $(LIB_SOURCES) $(LIB_HEADERS)
	@echo "$(YELLOW)Building memory benchmark with tracked allocations...$(NC)"
	@mkdir -p $(BENCH_DIR)/bin
	@mkdir -p $(BENCH_DIR)/include
	$(CC) $(CFLAGS_RELEASE) $< -DBENCHMARK_MEMORY_TRACKING \
		$(LIB_SOURCES) -I$(INC_DIR) -I$(BENCH_DIR)/include -o $@ $(LDFLAGS_RELEASE)

# Build benchmarks
build-benchmarks: $(BENCH_BINARIES)
	@echo "$(GREEN)✓ All benchmarks built$(NC)"

# Run benchmarks (new comprehensive system with git metadata)
benchmark: build-benchmarks
	@$(BENCH_DIR)/scripts/run_benchmark.sh

# Run performance benchmarks only
benchmark-perf: build-benchmarks
	@$(BENCH_DIR)/scripts/run_benchmark.sh --perf-only

# Run memory benchmarks only
benchmark-memory: build-benchmarks
	@$(BENCH_DIR)/scripts/run_benchmark.sh --memory-only

# Open visualization in browser (starts local server to avoid CORS issues)
benchmark-view:
	@$(BENCH_DIR)/scripts/serve_visualization.sh

# Legacy benchmark targets (kept for compatibility)
benchmark-legacy: build-benchmarks
	@echo "$(BLUE)=========================================$(NC)"
	@echo "$(BLUE)Running Benchmarks (Legacy)$(NC)"
	@echo "$(BLUE)=========================================$(NC)"
	@cd $(BENCH_DIR) && ./run_benchmarks.sh
	@echo ""
	@cd $(BENCH_DIR) && ./run_memory_benchmarks.sh

# ============================================================================
# Analysis & Information
# ============================================================================

# Show library information
info: libs
	@echo "$(BLUE)=========================================$(NC)"
	@echo "$(BLUE)Library Information$(NC)"
	@echo "$(BLUE)=========================================$(NC)"
	@echo ""
	@echo "$(YELLOW)Static Library:$(NC) $(STATIC_LIB)"
	@ls -lh $(STATIC_LIB) | awk '{print "  Size: " $$5}'
	@size $(STATIC_LIB)
	@echo ""
	@echo "$(YELLOW)Shared Library:$(NC) $(SHARED_LIB)"
	@ls -lh $(SHARED_LIB) | awk '{print "  Size: " $$5}'
	@size $(SHARED_LIB)
	@echo ""
	@echo "$(YELLOW)Object Files:$(NC)"
	@ls -lh $(LIB_OBJECTS) | awk '{print "  " $$9 ": " $$5}'
	@echo ""
	@echo "$(YELLOW)Symbols (functions):$(NC)"
	@nm $(STATIC_LIB) | grep ' T ' | head -20
	@echo ""

# Compare binary sizes across build types
check-size:
	@echo "$(BLUE)=========================================$(NC)"
	@echo "$(BLUE)Binary Size Comparison$(NC)"
	@echo "$(BLUE)=========================================$(NC)"
	@echo ""
	@echo "$(YELLOW)Building all variants...$(NC)"
	@$(MAKE) clean > /dev/null 2>&1
	@mkdir -p $(BUILD_DIR) $(LIB_DIR)
	@echo ""
	@echo "$(CYAN)1. Debug build (-O0 -g)$(NC)"
	@$(MAKE) debug CFLAGS="$(CFLAGS_DEBUG)" > /dev/null 2>&1
	@cp $(STATIC_LIB) $(BUILD_DIR)/lib_debug.a
	@cp $(SHARED_LIB) $(BUILD_DIR)/lib_debug.so
	@ls -lh $(BUILD_DIR)/lib_debug.a | awk '{print "   Static:  " $$5}'
	@ls -lh $(BUILD_DIR)/lib_debug.so | awk '{print "   Shared:  " $$5}'
	@echo ""
	@echo "$(CYAN)2. Release build (-O3 -flto -march=native)$(NC)"
	@$(MAKE) clean > /dev/null 2>&1
	@mkdir -p $(BUILD_DIR) $(LIB_DIR)
	@$(MAKE) release > /dev/null 2>&1
	@cp $(STATIC_LIB) $(BUILD_DIR)/lib_release.a
	@cp $(SHARED_LIB) $(BUILD_DIR)/lib_release.so
	@ls -lh $(BUILD_DIR)/lib_release.a | awk '{print "   Static:  " $$5}'
	@ls -lh $(BUILD_DIR)/lib_release.so | awk '{print "   Shared:  " $$5}'
	@echo ""
	@echo "$(CYAN)3. Size-optimized build (-Os)$(NC)"
	@$(MAKE) clean > /dev/null 2>&1
	@mkdir -p $(BUILD_DIR) $(LIB_DIR)
	@$(MAKE) size > /dev/null 2>&1
	@cp $(STATIC_LIB) $(BUILD_DIR)/lib_size.a
	@cp $(SHARED_LIB) $(BUILD_DIR)/lib_size.so
	@ls -lh $(BUILD_DIR)/lib_size.a | awk '{print "   Static:  " $$5}'
	@ls -lh $(BUILD_DIR)/lib_size.so | awk '{print "   Shared:  " $$5}'
	@echo ""
	@echo "$(GREEN)✓ Size comparison complete$(NC)"
	@echo "$(YELLOW)Comparison files saved in $(BUILD_DIR)/$(NC)"

# ============================================================================
# Installation
# ============================================================================

PREFIX ?= /usr/local
INSTALL_LIB_DIR = $(PREFIX)/lib
INSTALL_INC_DIR = $(PREFIX)/include/$(PROJECT_NAME)

install: release
	@echo "$(YELLOW)Installing to $(PREFIX)...$(NC)"
	@mkdir -p $(INSTALL_LIB_DIR)
	@mkdir -p $(INSTALL_INC_DIR)
	@cp $(STATIC_LIB) $(INSTALL_LIB_DIR)/
	@cp $(SHARED_LIB) $(INSTALL_LIB_DIR)/
	@cp $(LIB_HEADERS) $(INSTALL_INC_DIR)/
	@echo "$(GREEN)✓ Installation complete$(NC)"
	@echo ""
	@echo "Library files:"
	@echo "  $(INSTALL_LIB_DIR)/lib$(PROJECT_NAME).a"
	@echo "  $(INSTALL_LIB_DIR)/lib$(PROJECT_NAME).so"
	@echo ""
	@echo "Header files:"
	@echo "  $(INSTALL_INC_DIR)/*.h"
	@echo ""
	@echo "To use in your projects:"
	@echo "  $(CYAN)gcc your_program.c -l$(PROJECT_NAME) -o your_program$(NC)"

uninstall:
	@echo "$(YELLOW)Uninstalling from $(PREFIX)...$(NC)"
	@rm -f $(INSTALL_LIB_DIR)/lib$(PROJECT_NAME).a
	@rm -f $(INSTALL_LIB_DIR)/lib$(PROJECT_NAME).so
	@rm -rf $(INSTALL_INC_DIR)
	@echo "$(GREEN)✓ Uninstallation complete$(NC)"

# ============================================================================
# Cleanup
# ============================================================================

clean:
	@echo "$(YELLOW)Cleaning build artifacts...$(NC)"
	@rm -rf $(BUILD_DIR)
	@rm -rf $(LIB_DIR)
	@rm -rf $(DIST_DIR)
	@rm -f $(BENCH_BINARIES)
	@echo "$(GREEN)✓ Clean complete$(NC)"

# Create directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	@mkdir -p $(LIB_DIR)

$(DIST_DIR):
	@mkdir -p $(DIST_DIR)

# ============================================================================
# Development helpers
# ============================================================================

# Format code (requires clang-format)
format:
	@echo "$(YELLOW)Formatting code...$(NC)"
	@clang-format -i $(SRC_DIR)/*.c $(INC_DIR)/*.h $(TEST_DIR)/*.c || echo "$(RED)clang-format not found$(NC)"
	@echo "$(GREEN)✓ Format complete$(NC)"

# Run static analysis (requires cppcheck)
analyze:
	@echo "$(YELLOW)Running static analysis...$(NC)"
	@cppcheck --enable=all --suppress=missingIncludeSystem $(SRC_DIR)/ $(INC_DIR)/ || echo "$(RED)cppcheck not found$(NC)"

# Show TODO items in code
todos:
	@echo "$(YELLOW)TODO items in code:$(NC)"
	@grep -rn "TODO" $(SRC_DIR)/ $(INC_DIR)/ $(TEST_DIR)/ || echo "No TODOs found"

# ============================================================================
# Special targets
# ============================================================================

.DEFAULT_GOAL := all
.PRECIOUS: $(BUILD_DIR)/%.o
