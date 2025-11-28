# JSON Parser Optimization Guide

## Executive Summary

This document provides a comprehensive analysis of the current JSON parser implementation and outlines optimization strategies to achieve performance competitive with or exceeding Node.js V8's native JSON parser.

### Current Performance Baseline

Based on benchmark results (commit `249d313`):

| Metric | C Parser | Node.js V8 | Ratio |
|--------|----------|------------|-------|
| large.json (13.9MB) | 398 MB/s | 733 MB/s | 0.54x |
| xlarge.json (39.7MB) | 381 MB/s | 339 MB/s | 1.12x |
| long_strings.json | 911 MB/s | 2299 MB/s | 0.40x |
| large_object.json | 257 MB/s | 295 MB/s | 0.87x |

**Key Insight**: The parser already beats V8 on very large files (`xlarge.json`) but struggles with string-heavy workloads and medium-sized files.

---

## Table of Contents

1. [Architecture Analysis](#1-architecture-analysis)
2. [Memory Pool Allocator](#2-memory-pool-allocator)
3. [SIMD Optimizations](#3-simd-optimizations)
4. [Lazy Hash Table](#4-lazy-hash-table)
5. [String Interning](#5-string-interning)
6. [Zero-Copy Parsing](#6-zero-copy-parsing)
7. [Branch Prediction Optimization](#7-branch-prediction-optimization)
8. [Cache-Friendly Data Structures](#8-cache-friendly-data-structures)
9. [Benchmarking Strategy](#9-benchmarking-strategy)
10. [Implementation Roadmap](#10-implementation-roadmap)

---

## 1. Architecture Analysis

### 1.1 Current Architecture Overview

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Input     │────▶│   Lexer     │────▶│   Parser    │
│  (string)   │     │ (tokenize)  │     │ (AST build) │
└─────────────┘     └─────────────┘     └─────────────┘
                           │                   │
                           ▼                   ▼
                    ┌─────────────┐     ┌─────────────┐
                    │   Token     │     │ json_value_t│
                    │  (struct)   │     │   (union)   │
                    └─────────────┘     └─────────────┘
```

### 1.2 Identified Bottlenecks

#### Memory Allocation Overhead

**Current Pattern** (from `lexer.c` and `json.c`):
```c
// lexer_init: Allocates copy of entire input
lexer_t lexer_init(const char *input) {
    size_t len = strlen(input);
    char *in_str = malloc(len + 1);  // Allocation #1
    strcpy(in_str, input);
    // ...
}

// slice_to_string: Allocates for every string token
char *slice_to_string(string_slice_t slice) {
    char *str = malloc(slice.length + 1);  // Allocation per string
    memcpy(str, slice.start, slice.length);
    str[slice.length] = '\0';
    return str;
}

// hash_table_insert: Allocates key copy
char *key_copy = malloc(key_len + 1);  // Allocation per object key
```

**Impact**: For `large.json` (13.9MB), benchmark shows:
- 98,501,000 malloc calls
- 105,001,100 free calls
- Heavy allocator pressure

#### String Processing Bottleneck

**Current tokenize_string**:
```c
token_t tokenize_string(lexer_t *lexer) {
    // Character-by-character processing
    while (*lexer->current != '\0' && *lexer->current != '"') {
        if (*lexer->current == '\\') {
            lexer->current++;  // Skip escape
            lexer->column++;
            if (*lexer->current != '\0') {
                lexer->current++;
                lexer->column++;
            }
        } else {
            // Single byte at a time
            lexer->current++;
        }
    }
}
```

**Impact**: String-heavy files (`long_strings.json`) show 2.5x slower than V8.

#### Hash Table Inefficiency

**Current Implementation**:
- Separate chaining with dynamic bucket arrays
- Immediate resize at 75% load factor
- Key comparison requires full string comparison

```c
// Every lookup does full string comparison
for (size_t i = 0; i < bucket->len; i++) {
    if (bucket->items[i].key_len == key_len &&
        memcmp(bucket->items[i].key, key, key_len) == 0) {
        return &bucket->items[i].value;
    }
}
```

---

## 2. Memory Pool Allocator

### 2.1 Design Overview

A memory pool (arena allocator) pre-allocates large chunks and hands out memory sequentially, eliminating per-allocation overhead.

```
┌────────────────────────────────────────────────────────┐
│                    Memory Pool                          │
├────────────────────────────────────────────────────────┤
│ Block 1 (1MB)                                          │
│ ┌──────┬──────┬──────┬──────┬─────────────────────────┐│
│ │obj 1 │obj 2 │obj 3 │obj 4 │      free space        ││
│ └──────┴──────┴──────┴──────┴─────────────────────────┘│
├────────────────────────────────────────────────────────┤
│ Block 2 (1MB) - allocated when Block 1 fills          │
│ ┌──────────────────────────────────────────────────────┐│
│ │                    free space                        ││
│ └──────────────────────────────────────────────────────┘│
└────────────────────────────────────────────────────────┘
```

### 2.2 Implementation

```c
// include/mem_pool.h

#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define POOL_BLOCK_SIZE (1024 * 1024)  // 1MB blocks
#define POOL_ALIGNMENT 8

typedef struct pool_block {
    struct pool_block *next;
    size_t size;
    size_t used;
    char data[];  // Flexible array member
} pool_block_t;

typedef struct {
    pool_block_t *current;
    pool_block_t *head;
    size_t total_allocated;
    size_t total_used;
    size_t block_count;
} mem_pool_t;

// Initialize a new memory pool
mem_pool_t *pool_create(void);

// Allocate memory from pool (fast path: bump pointer)
void *pool_alloc(mem_pool_t *pool, size_t size);

// Allocate aligned memory
void *pool_alloc_aligned(mem_pool_t *pool, size_t size, size_t alignment);

// Reset pool for reuse (keeps allocated blocks)
void pool_reset(mem_pool_t *pool);

// Destroy pool and free all memory
void pool_destroy(mem_pool_t *pool);

// Statistics
size_t pool_bytes_used(mem_pool_t *pool);
size_t pool_bytes_allocated(mem_pool_t *pool);

#endif // MEM_POOL_H
```

```c
// src/mem_pool.c

#include "mem_pool.h"
#include <stdlib.h>
#include <string.h>

// Align size up to alignment boundary
static inline size_t align_up(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

static pool_block_t *block_create(size_t min_size) {
    size_t block_size = min_size > POOL_BLOCK_SIZE ? min_size : POOL_BLOCK_SIZE;
    pool_block_t *block = malloc(sizeof(pool_block_t) + block_size);
    if (!block) return NULL;
    
    block->next = NULL;
    block->size = block_size;
    block->used = 0;
    return block;
}

mem_pool_t *pool_create(void) {
    mem_pool_t *pool = malloc(sizeof(mem_pool_t));
    if (!pool) return NULL;
    
    pool->head = block_create(POOL_BLOCK_SIZE);
    if (!pool->head) {
        free(pool);
        return NULL;
    }
    
    pool->current = pool->head;
    pool->total_allocated = POOL_BLOCK_SIZE;
    pool->total_used = 0;
    pool->block_count = 1;
    return pool;
}

// Hot path - inline for performance
void *pool_alloc(mem_pool_t *pool, size_t size) {
    size = align_up(size, POOL_ALIGNMENT);
    
    // Fast path: allocation fits in current block
    if (pool->current->used + size <= pool->current->size) {
        void *ptr = pool->current->data + pool->current->used;
        pool->current->used += size;
        pool->total_used += size;
        return ptr;
    }
    
    // Slow path: need new block
    pool_block_t *new_block = block_create(size);
    if (!new_block) return NULL;
    
    new_block->next = NULL;
    pool->current->next = new_block;
    pool->current = new_block;
    pool->total_allocated += new_block->size;
    pool->block_count++;
    
    void *ptr = new_block->data;
    new_block->used = size;
    pool->total_used += size;
    return ptr;
}

void *pool_alloc_aligned(mem_pool_t *pool, size_t size, size_t alignment) {
    // Ensure alignment is power of 2
    size_t padding = (alignment - ((uintptr_t)(pool->current->data + pool->current->used) & (alignment - 1))) & (alignment - 1);
    return pool_alloc(pool, size + padding);
}

void pool_reset(mem_pool_t *pool) {
    // Reset all blocks but keep them allocated
    pool_block_t *block = pool->head;
    while (block) {
        block->used = 0;
        block = block->next;
    }
    pool->current = pool->head;
    pool->total_used = 0;
}

void pool_destroy(mem_pool_t *pool) {
    pool_block_t *block = pool->head;
    while (block) {
        pool_block_t *next = block->next;
        free(block);
        block = next;
    }
    free(pool);
}

size_t pool_bytes_used(mem_pool_t *pool) {
    return pool->total_used;
}

size_t pool_bytes_allocated(mem_pool_t *pool) {
    return pool->total_allocated;
}
```

### 2.3 Integration with Parser

```c
// Modified parser_t structure
typedef struct {
    lexer_t *lexer;
    token_t current_token;
    bool has_error;
    char error_message[256];
    mem_pool_t *pool;  // Add pool reference
} parser_t;

// Modified parser initialization
parser_t parser_init_with_pool(lexer_t *lexer, mem_pool_t *pool) {
    parser_t parser = {
        .lexer = lexer,
        .pool = pool,
        .has_error = false,
    };
    return parser;
}

// Pool-aware string allocation
char *pool_strdup(mem_pool_t *pool, const char *src, size_t len) {
    char *dst = pool_alloc(pool, len + 1);
    if (dst) {
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
    return dst;
}

// Modified parse_string
json_value_t parse_string_pooled(parser_t *parser) {
    if (!check(parser, TOKEN_STRING)) {
        parser_error(parser, "Expected string");
        return json_value_init(JSON_NULL);
    }
    
    // Allocate from pool instead of malloc
    string_slice_t slice = parser->current_token.lexeme;
    char *str = pool_strdup(parser->pool, slice.start, slice.length);
    
    json_value_t value = json_value_init(JSON_STRING);
    value.string = str;
    advance(parser);
    return value;
}
```

### 2.4 Expected Performance Impact

| Workload | Current | With Pool | Improvement |
|----------|---------|-----------|-------------|
| Malloc calls (large.json) | 98M | ~100 | 99.9999% reduction |
| Parse time (large.json) | 33.4ms | ~20ms | ~40% faster |
| Memory fragmentation | High | None | Eliminated |

---

## 3. SIMD Optimizations

### 3.1 Target Operations

SIMD (Single Instruction Multiple Data) can accelerate:
1. **Whitespace skipping** - Process 16-32 bytes at once
2. **String scanning** - Find `"` and `\` in parallel
3. **Number validation** - Validate digit sequences
4. **Keyword matching** - Compare `true`, `false`, `null` in one operation

### 3.2 Whitespace Skipping with SIMD

```c
// include/simd_utils.h

#ifndef SIMD_UTILS_H
#define SIMD_UTILS_H

#include <stddef.h>
#include <stdint.h>

#if defined(__SSE4_2__) || defined(__AVX2__)
    #define HAS_SIMD 1
    #include <immintrin.h>
#elif defined(__ARM_NEON)
    #define HAS_SIMD 1
    #include <arm_neon.h>
#else
    #define HAS_SIMD 0
#endif

// Skip whitespace using SIMD, returns pointer to first non-whitespace
const char *skip_whitespace_simd(const char *ptr, const char *end);

// Find end of string (unescaped quote), returns pointer to quote or backslash
const char *find_string_end_simd(const char *ptr, const char *end);

// Validate number characters, returns count of valid digits
size_t count_digits_simd(const char *ptr, const char *end);

#endif // SIMD_UTILS_H
```

```c
// src/simd_utils.c

#include "simd_utils.h"

#if HAS_SIMD && defined(__SSE4_2__)

// SSE4.2 implementation for x86-64

const char *skip_whitespace_simd(const char *ptr, const char *end) {
    // Handle unaligned prefix
    while (ptr < end && ((uintptr_t)ptr & 15)) {
        char c = *ptr;
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            return ptr;
        }
        ptr++;
    }
    
    // SIMD loop - process 16 bytes at a time
    __m128i space = _mm_set1_epi8(' ');
    __m128i tab = _mm_set1_epi8('\t');
    __m128i newline = _mm_set1_epi8('\n');
    __m128i cr = _mm_set1_epi8('\r');
    
    while (ptr + 16 <= end) {
        __m128i chunk = _mm_load_si128((const __m128i *)ptr);
        
        // Check if any byte is NOT whitespace
        __m128i is_space = _mm_cmpeq_epi8(chunk, space);
        __m128i is_tab = _mm_cmpeq_epi8(chunk, tab);
        __m128i is_nl = _mm_cmpeq_epi8(chunk, newline);
        __m128i is_cr = _mm_cmpeq_epi8(chunk, cr);
        
        __m128i is_ws = _mm_or_si128(
            _mm_or_si128(is_space, is_tab),
            _mm_or_si128(is_nl, is_cr)
        );
        
        int mask = _mm_movemask_epi8(is_ws);
        if (mask != 0xFFFF) {
            // Found non-whitespace
            // Find first non-whitespace position
            int non_ws_mask = ~mask & 0xFFFF;
            int pos = __builtin_ctz(non_ws_mask);
            return ptr + pos;
        }
        
        ptr += 16;
    }
    
    // Handle remaining bytes
    while (ptr < end) {
        char c = *ptr;
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            return ptr;
        }
        ptr++;
    }
    
    return ptr;
}

const char *find_string_end_simd(const char *ptr, const char *end) {
    __m128i quote = _mm_set1_epi8('"');
    __m128i backslash = _mm_set1_epi8('\\');
    
    while (ptr + 16 <= end) {
        __m128i chunk = _mm_loadu_si128((const __m128i *)ptr);
        
        __m128i has_quote = _mm_cmpeq_epi8(chunk, quote);
        __m128i has_bs = _mm_cmpeq_epi8(chunk, backslash);
        __m128i has_special = _mm_or_si128(has_quote, has_bs);
        
        int mask = _mm_movemask_epi8(has_special);
        if (mask != 0) {
            int pos = __builtin_ctz(mask);
            return ptr + pos;
        }
        
        ptr += 16;
    }
    
    // Scalar fallback for remaining bytes
    while (ptr < end) {
        if (*ptr == '"' || *ptr == '\\') {
            return ptr;
        }
        ptr++;
    }
    
    return ptr;
}

size_t count_digits_simd(const char *ptr, const char *end) {
    size_t count = 0;
    
    __m128i zero = _mm_set1_epi8('0');
    __m128i nine = _mm_set1_epi8('9');
    
    while (ptr + 16 <= end) {
        __m128i chunk = _mm_loadu_si128((const __m128i *)ptr);
        
        // Check if each byte is >= '0' and <= '9'
        __m128i ge_zero = _mm_cmpgt_epi8(chunk, _mm_sub_epi8(zero, _mm_set1_epi8(1)));
        __m128i le_nine = _mm_cmplt_epi8(chunk, _mm_add_epi8(nine, _mm_set1_epi8(1)));
        __m128i is_digit = _mm_and_si128(ge_zero, le_nine);
        
        int mask = _mm_movemask_epi8(is_digit);
        if (mask != 0xFFFF) {
            // Found non-digit, count leading digits
            int non_digit_pos = __builtin_ctz(~mask & 0xFFFF);
            return count + non_digit_pos;
        }
        
        count += 16;
        ptr += 16;
    }
    
    // Scalar fallback
    while (ptr < end && *ptr >= '0' && *ptr <= '9') {
        count++;
        ptr++;
    }
    
    return count;
}

#elif HAS_SIMD && defined(__ARM_NEON)

// ARM NEON implementation (for Apple Silicon, etc.)

const char *skip_whitespace_simd(const char *ptr, const char *end) {
    uint8x16_t space = vdupq_n_u8(' ');
    uint8x16_t tab = vdupq_n_u8('\t');
    uint8x16_t newline = vdupq_n_u8('\n');
    uint8x16_t cr = vdupq_n_u8('\r');
    
    while (ptr + 16 <= end) {
        uint8x16_t chunk = vld1q_u8((const uint8_t *)ptr);
        
        uint8x16_t is_space = vceqq_u8(chunk, space);
        uint8x16_t is_tab = vceqq_u8(chunk, tab);
        uint8x16_t is_nl = vceqq_u8(chunk, newline);
        uint8x16_t is_cr = vceqq_u8(chunk, cr);
        
        uint8x16_t is_ws = vorrq_u8(
            vorrq_u8(is_space, is_tab),
            vorrq_u8(is_nl, is_cr)
        );
        
        // Check if all bytes are whitespace
        uint64x2_t v64 = vreinterpretq_u64_u8(is_ws);
        if (vgetq_lane_u64(v64, 0) != ~0ULL || vgetq_lane_u64(v64, 1) != ~0ULL) {
            // Found non-whitespace, find position
            for (int i = 0; i < 16 && ptr + i < end; i++) {
                char c = ptr[i];
                if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                    return ptr + i;
                }
            }
        }
        
        ptr += 16;
    }
    
    // Scalar fallback
    while (ptr < end) {
        char c = *ptr;
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            return ptr;
        }
        ptr++;
    }
    
    return ptr;
}

const char *find_string_end_simd(const char *ptr, const char *end) {
    uint8x16_t quote = vdupq_n_u8('"');
    uint8x16_t backslash = vdupq_n_u8('\\');
    
    while (ptr + 16 <= end) {
        uint8x16_t chunk = vld1q_u8((const uint8_t *)ptr);
        
        uint8x16_t has_quote = vceqq_u8(chunk, quote);
        uint8x16_t has_bs = vceqq_u8(chunk, backslash);
        uint8x16_t has_special = vorrq_u8(has_quote, has_bs);
        
        // Check if any special character found
        uint64x2_t v64 = vreinterpretq_u64_u8(has_special);
        if (vgetq_lane_u64(v64, 0) != 0 || vgetq_lane_u64(v64, 1) != 0) {
            for (int i = 0; i < 16; i++) {
                if (ptr[i] == '"' || ptr[i] == '\\') {
                    return ptr + i;
                }
            }
        }
        
        ptr += 16;
    }
    
    // Scalar fallback
    while (ptr < end) {
        if (*ptr == '"' || *ptr == '\\') {
            return ptr;
        }
        ptr++;
    }
    
    return ptr;
}

size_t count_digits_simd(const char *ptr, const char *end) {
    size_t count = 0;
    
    uint8x16_t zero = vdupq_n_u8('0');
    uint8x16_t ten = vdupq_n_u8(10);
    
    while (ptr + 16 <= end) {
        uint8x16_t chunk = vld1q_u8((const uint8_t *)ptr);
        uint8x16_t normalized = vsubq_u8(chunk, zero);
        uint8x16_t is_digit = vcltq_u8(normalized, ten);
        
        // Check if all are digits
        uint64x2_t v64 = vreinterpretq_u64_u8(is_digit);
        if (vgetq_lane_u64(v64, 0) != ~0ULL || vgetq_lane_u64(v64, 1) != ~0ULL) {
            for (int i = 0; i < 16; i++) {
                if (ptr[i] < '0' || ptr[i] > '9') {
                    return count + i;
                }
            }
        }
        
        count += 16;
        ptr += 16;
    }
    
    // Scalar fallback
    while (ptr < end && *ptr >= '0' && *ptr <= '9') {
        count++;
        ptr++;
    }
    
    return count;
}

#else

// Scalar fallback for systems without SIMD

const char *skip_whitespace_simd(const char *ptr, const char *end) {
    while (ptr < end) {
        char c = *ptr;
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            return ptr;
        }
        ptr++;
    }
    return ptr;
}

const char *find_string_end_simd(const char *ptr, const char *end) {
    while (ptr < end) {
        if (*ptr == '"' || *ptr == '\\') {
            return ptr;
        }
        ptr++;
    }
    return ptr;
}

size_t count_digits_simd(const char *ptr, const char *end) {
    size_t count = 0;
    while (ptr < end && *ptr >= '0' && *ptr <= '9') {
        count++;
        ptr++;
    }
    return count;
}

#endif
```

### 3.3 Optimized String Tokenization

```c
// Optimized tokenize_string using SIMD
token_t tokenize_string_simd(lexer_t *lexer) {
    token_t token;
    const char *end = lexer->start + strlen(lexer->start);
    
    // Skip opening quote
    lexer->current++;
    lexer->column++;
    const char *start = lexer->current;
    
    while (lexer->current < end) {
        // Use SIMD to find next special character
        const char *special = find_string_end_simd(lexer->current, end);
        
        // Advance to special character
        size_t skip = special - lexer->current;
        lexer->column += skip;
        lexer->current = special;
        
        if (lexer->current >= end || *lexer->current == '\0') {
            // Unterminated string
            token.type = TOKEN_ERROR;
            token.lexeme.start = "Unterminated string";
            token.lexeme.length = 19;
            return token;
        }
        
        if (*lexer->current == '"') {
            // Found end of string
            break;
        }
        
        // Handle escape sequence
        if (*lexer->current == '\\') {
            lexer->current++;
            lexer->column++;
            if (lexer->current < end && *lexer->current != '\0') {
                lexer->current++;
                lexer->column++;
            }
        }
    }
    
    size_t len = lexer->current - start;
    token.type = TOKEN_STRING;
    token.line = lexer->line;
    token.column = lexer->column - len - 1;
    token.lexeme.start = start;
    token.lexeme.length = len;
    
    // Skip closing quote
    lexer->current++;
    lexer->column++;
    
    return token;
}
```

### 3.4 Expected Performance Impact

| Operation | Scalar (ns) | SIMD (ns) | Speedup |
|-----------|-------------|-----------|---------|
| Skip 64 bytes whitespace | ~64 | ~8 | 8x |
| Find string end (no escapes) | ~100 | ~12 | 8x |
| Validate 32 digits | ~32 | ~4 | 8x |

**Overall parser improvement for string-heavy workloads**: 30-50%

---

## 4. Lazy Hash Table

### 4.1 Concept

Instead of building the hash table during parsing, store key-value pairs in a flat array and build the hash table only when lookup is needed (or never, if keys are accessed sequentially).

```
┌─────────────────────────────────────────────────────────┐
│                   Lazy Object                            │
├─────────────────────────────────────────────────────────┤
│ Stage 1: Array-backed (during parsing)                  │
│ ┌────────┬────────┬────────┬────────┬────────┐         │
│ │ entry1 │ entry2 │ entry3 │ entry4 │  ...   │         │
│ └────────┴────────┴────────┴────────┴────────┘         │
│                                                         │
│ Stage 2: Hash table (built on first random access)     │
│ ┌────────────────────────────────────────────┐         │
│ │ bucket[0] ──▶ entry2                       │         │
│ │ bucket[1] ──▶ entry1 ──▶ entry4            │         │
│ │ bucket[2] ──▶ entry3                       │         │
│ │ ...                                        │         │
│ └────────────────────────────────────────────┘         │
└─────────────────────────────────────────────────────────┘
```

### 4.2 Implementation

```c
// include/lazy_object.h

#ifndef LAZY_OBJECT_H
#define LAZY_OBJECT_H

#include "json.h"
#include "mem_pool.h"

#define LAZY_THRESHOLD 8  // Build hash table for objects > 8 entries

typedef struct {
    const char *key;      // Points into source (zero-copy)
    size_t key_len;
    uint32_t key_hash;    // Pre-computed hash
    json_value_t value;
} lazy_entry_t;

typedef struct {
    // Flat array storage (always populated)
    lazy_entry_t *entries;
    size_t len;
    size_t cap;
    
    // Hash table (lazily built)
    uint32_t *hash_index;  // Maps hash -> entry index
    size_t hash_cap;
    bool hash_built;
    
    // Memory pool reference
    mem_pool_t *pool;
} lazy_object_t;

// Create lazy object with initial capacity
lazy_object_t *lazy_object_create(mem_pool_t *pool, size_t initial_cap);

// Add entry during parsing (O(1) amortized)
int lazy_object_add(lazy_object_t *obj, const char *key, size_t key_len, json_value_t value);

// Get value by key (builds hash on first call for large objects)
json_value_t *lazy_object_get(lazy_object_t *obj, const char *key, size_t key_len);

// Check if key exists
bool lazy_object_has(lazy_object_t *obj, const char *key, size_t key_len);

// Get entry count
size_t lazy_object_size(lazy_object_t *obj);

// Iterate entries (for serialization, etc.)
typedef void (*lazy_object_visitor)(const char *key, size_t key_len, json_value_t *value, void *ctx);
void lazy_object_iterate(lazy_object_t *obj, lazy_object_visitor visitor, void *ctx);

#endif // LAZY_OBJECT_H
```

```c
// src/lazy_object.c

#include "lazy_object.h"
#include <string.h>

// FNV-1a hash (same as current implementation)
static inline uint32_t hash_key(const char *key, size_t len) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619u;
    }
    return hash;
}

lazy_object_t *lazy_object_create(mem_pool_t *pool, size_t initial_cap) {
    lazy_object_t *obj = pool_alloc(pool, sizeof(lazy_object_t));
    if (!obj) return NULL;
    
    initial_cap = initial_cap < 4 ? 4 : initial_cap;
    
    obj->entries = pool_alloc(pool, sizeof(lazy_entry_t) * initial_cap);
    obj->len = 0;
    obj->cap = initial_cap;
    obj->hash_index = NULL;
    obj->hash_cap = 0;
    obj->hash_built = false;
    obj->pool = pool;
    
    return obj;
}

int lazy_object_add(lazy_object_t *obj, const char *key, size_t key_len, json_value_t value) {
    // Grow array if needed
    if (obj->len >= obj->cap) {
        size_t new_cap = obj->cap * 2;
        lazy_entry_t *new_entries = pool_alloc(obj->pool, sizeof(lazy_entry_t) * new_cap);
        if (!new_entries) return -1;
        
        memcpy(new_entries, obj->entries, sizeof(lazy_entry_t) * obj->len);
        obj->entries = new_entries;
        obj->cap = new_cap;
    }
    
    // Add entry to array
    lazy_entry_t *entry = &obj->entries[obj->len++];
    entry->key = key;
    entry->key_len = key_len;
    entry->key_hash = hash_key(key, key_len);
    entry->value = value;
    
    // Invalidate hash table if it was built
    obj->hash_built = false;
    
    return 0;
}

static void build_hash_table(lazy_object_t *obj) {
    if (obj->hash_built || obj->len <= LAZY_THRESHOLD) {
        return;
    }
    
    // Use power-of-2 sizing for fast modulo
    size_t hash_cap = 16;
    while (hash_cap < obj->len * 2) {
        hash_cap *= 2;
    }
    
    obj->hash_index = pool_alloc(obj->pool, sizeof(uint32_t) * hash_cap);
    if (!obj->hash_index) return;
    
    obj->hash_cap = hash_cap;
    
    // Initialize to invalid index
    memset(obj->hash_index, 0xFF, sizeof(uint32_t) * hash_cap);
    
    // Insert all entries using linear probing
    for (size_t i = 0; i < obj->len; i++) {
        uint32_t idx = obj->entries[i].key_hash & (hash_cap - 1);
        
        // Linear probe for empty slot
        while (obj->hash_index[idx] != 0xFFFFFFFF) {
            idx = (idx + 1) & (hash_cap - 1);
        }
        
        obj->hash_index[idx] = i;
    }
    
    obj->hash_built = true;
}

json_value_t *lazy_object_get(lazy_object_t *obj, const char *key, size_t key_len) {
    // For small objects, linear search is faster
    if (obj->len <= LAZY_THRESHOLD) {
        uint32_t key_hash = hash_key(key, key_len);
        
        for (size_t i = 0; i < obj->len; i++) {
            if (obj->entries[i].key_hash == key_hash &&
                obj->entries[i].key_len == key_len &&
                memcmp(obj->entries[i].key, key, key_len) == 0) {
                return &obj->entries[i].value;
            }
        }
        return NULL;
    }
    
    // Build hash table if not already built
    if (!obj->hash_built) {
        build_hash_table(obj);
    }
    
    if (!obj->hash_index) {
        // Hash build failed, fallback to linear search
        for (size_t i = 0; i < obj->len; i++) {
            if (obj->entries[i].key_len == key_len &&
                memcmp(obj->entries[i].key, key, key_len) == 0) {
                return &obj->entries[i].value;
            }
        }
        return NULL;
    }
    
    // Hash table lookup with linear probing
    uint32_t key_hash = hash_key(key, key_len);
    uint32_t idx = key_hash & (obj->hash_cap - 1);
    
    while (obj->hash_index[idx] != 0xFFFFFFFF) {
        uint32_t entry_idx = obj->hash_index[idx];
        lazy_entry_t *entry = &obj->entries[entry_idx];
        
        if (entry->key_hash == key_hash &&
            entry->key_len == key_len &&
            memcmp(entry->key, key, key_len) == 0) {
            return &entry->value;
        }
        
        idx = (idx + 1) & (obj->hash_cap - 1);
    }
    
    return NULL;
}

bool lazy_object_has(lazy_object_t *obj, const char *key, size_t key_len) {
    return lazy_object_get(obj, key, key_len) != NULL;
}

size_t lazy_object_size(lazy_object_t *obj) {
    return obj->len;
}

void lazy_object_iterate(lazy_object_t *obj, lazy_object_visitor visitor, void *ctx) {
    for (size_t i = 0; i < obj->len; i++) {
        lazy_entry_t *entry = &obj->entries[i];
        visitor(entry->key, entry->key_len, &entry->value, ctx);
    }
}
```

### 4.3 Expected Performance Impact

| Scenario | Current | With Lazy | Improvement |
|----------|---------|-----------|-------------|
| Parse-only (no access) | 100% | 60% | 40% faster |
| Parse + sequential access | 100% | 70% | 30% faster |
| Parse + random access | 100% | 95% | 5% faster |
| Small objects (<8 keys) | 100% | 80% | 20% faster |

---

## 5. String Interning

### 5.1 Concept

Many JSON documents have repeated strings (field names, enum values). String interning stores each unique string once and returns pointers to the interned copy.

```
┌─────────────────────────────────────────────────────────┐
│                   String Intern Table                    │
├─────────────────────────────────────────────────────────┤
│ Hash: 0x1234 ──▶ "name"                                 │
│ Hash: 0x5678 ──▶ "age"                                  │
│ Hash: 0x9ABC ──▶ "email"                                │
│ Hash: 0xDEF0 ──▶ "active"                               │
│                                                         │
│ All json_value_t with "name" point to same address ──▶ │
└─────────────────────────────────────────────────────────┘
```

### 5.2 Implementation

```c
// include/string_intern.h

#ifndef STRING_INTERN_H
#define STRING_INTERN_H

#include "mem_pool.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct intern_entry {
    const char *str;
    size_t len;
    uint32_t hash;
    struct intern_entry *next;  // For chaining
} intern_entry_t;

typedef struct {
    intern_entry_t **buckets;
    size_t bucket_count;
    size_t entry_count;
    mem_pool_t *pool;
} string_intern_t;

// Create intern table
string_intern_t *intern_create(mem_pool_t *pool, size_t initial_buckets);

// Intern a string - returns pointer to interned copy
// If string already exists, returns existing pointer
// If new, copies to pool and returns new pointer
const char *intern_string(string_intern_t *table, const char *str, size_t len);

// Check if string is already interned (without interning it)
const char *intern_lookup(string_intern_t *table, const char *str, size_t len);

// Get statistics
size_t intern_unique_count(string_intern_t *table);
size_t intern_total_bytes_saved(string_intern_t *table);

#endif // STRING_INTERN_H
```

```c
// src/string_intern.c

#include "string_intern.h"
#include <string.h>

static uint32_t hash_string(const char *str, size_t len) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619u;
    }
    return hash;
}

string_intern_t *intern_create(mem_pool_t *pool, size_t initial_buckets) {
    string_intern_t *table = pool_alloc(pool, sizeof(string_intern_t));
    if (!table) return NULL;
    
    // Round up to power of 2
    size_t buckets = 64;
    while (buckets < initial_buckets) {
        buckets *= 2;
    }
    
    table->buckets = pool_alloc(pool, sizeof(intern_entry_t *) * buckets);
    if (!table->buckets) return NULL;
    
    memset(table->buckets, 0, sizeof(intern_entry_t *) * buckets);
    table->bucket_count = buckets;
    table->entry_count = 0;
    table->pool = pool;
    
    return table;
}

const char *intern_string(string_intern_t *table, const char *str, size_t len) {
    uint32_t hash = hash_string(str, len);
    size_t idx = hash & (table->bucket_count - 1);
    
    // Search existing entries
    intern_entry_t *entry = table->buckets[idx];
    while (entry) {
        if (entry->hash == hash && 
            entry->len == len && 
            memcmp(entry->str, str, len) == 0) {
            return entry->str;  // Already interned
        }
        entry = entry->next;
    }
    
    // Need to intern new string
    char *interned = pool_alloc(table->pool, len + 1);
    if (!interned) return NULL;
    
    memcpy(interned, str, len);
    interned[len] = '\0';
    
    intern_entry_t *new_entry = pool_alloc(table->pool, sizeof(intern_entry_t));
    if (!new_entry) return NULL;
    
    new_entry->str = interned;
    new_entry->len = len;
    new_entry->hash = hash;
    new_entry->next = table->buckets[idx];
    table->buckets[idx] = new_entry;
    table->entry_count++;
    
    return interned;
}

const char *intern_lookup(string_intern_t *table, const char *str, size_t len) {
    uint32_t hash = hash_string(str, len);
    size_t idx = hash & (table->bucket_count - 1);
    
    intern_entry_t *entry = table->buckets[idx];
    while (entry) {
        if (entry->hash == hash && 
            entry->len == len && 
            memcmp(entry->str, str, len) == 0) {
            return entry->str;
        }
        entry = entry->next;
    }
    
    return NULL;
}

size_t intern_unique_count(string_intern_t *table) {
    return table->entry_count;
}
```

### 5.3 Usage in Parser

```c
// Modified parser to use interning for object keys
typedef struct {
    lexer_t *lexer;
    token_t current_token;
    bool has_error;
    char error_message[256];
    mem_pool_t *pool;
    string_intern_t *intern;  // Add intern table
} parser_t;

json_value_t parse_object_interned(parser_t *parser) {
    // ... setup code ...
    
    while (true) {
        // Parse key using interning
        string_slice_t key_slice = parser->current_token.lexeme;
        const char *key = intern_string(parser->intern, key_slice.start, key_slice.length);
        
        advance(parser);
        // ... rest of parsing ...
        
        // Key is now interned - no allocation for duplicate keys
        lazy_object_add(obj, key, key_slice.length, value);
    }
}
```

### 5.4 Expected Performance Impact

For documents with repeated keys (common in arrays of objects):

| Document Type | Memory Saved | Parse Time Improvement |
|---------------|--------------|------------------------|
| Homogeneous array (1000 objects, 10 keys) | 90% key storage | 15-20% |
| Config file (few repeated keys) | 10-20% key storage | 2-5% |
| Deeply nested (unique keys) | 0% | ~0% |

---

## 6. Zero-Copy Parsing

### 6.1 Concept

Instead of copying strings from input to json_value_t, store pointers into the original input buffer. This requires the input to remain valid for the lifetime of the parsed structure.

```
┌─────────────────────────────────────────────────────────┐
│ Input Buffer: {"name": "John", "age": 30}               │
│               ^      ^  ^     ^                         │
│               │      │  │     │                         │
│               │      │  └─────┴─── json_value_t.string  │
│               │      └──────────── key pointer          │
│               └─────────────────── json_value_t.string  │
└─────────────────────────────────────────────────────────┘
```

### 6.2 Implementation

```c
// include/json_zerocopy.h

#ifndef JSON_ZEROCOPY_H
#define JSON_ZEROCOPY_H

#include <stddef.h>
#include <stdbool.h>

// Zero-copy string view (no ownership)
typedef struct {
    const char *data;  // Points into input buffer
    size_t len;
} json_string_view_t;

// Flag to indicate if string is owned or view
#define JSON_STRING_FLAG_VIEW  0x01
#define JSON_STRING_FLAG_OWNED 0x02
#define JSON_STRING_FLAG_ESCAPED 0x04  // Contains escape sequences

typedef struct json_value_zc {
    json_type_t type;
    uint8_t flags;  // For strings: VIEW, OWNED, ESCAPED
    union {
        double number;
        bool boolean;
        struct {
            union {
                char *owned;           // Owned string (if escaped)
                json_string_view_t view;  // View into input (if no escapes)
            };
        } string;
        struct {
            struct json_value_zc *items;
            size_t len;
            size_t cap;
        } array;
        struct lazy_object *object;
    };
} json_value_zc_t;

// Get string data (handles both owned and view)
static inline const char *json_get_string(json_value_zc_t *val, size_t *len) {
    if (val->flags & JSON_STRING_FLAG_VIEW) {
        *len = val->string.view.len;
        return val->string.view.data;
    } else {
        *len = strlen(val->string.owned);
        return val->string.owned;
    }
}

#endif // JSON_ZEROCOPY_H
```

### 6.3 Modified String Parsing

```c
// Zero-copy string parsing
json_value_zc_t parse_string_zerocopy(parser_t *parser) {
    json_value_zc_t value;
    value.type = JSON_STRING;
    
    string_slice_t slice = parser->current_token.lexeme;
    
    // Check if string contains escape sequences
    bool has_escapes = false;
    for (size_t i = 0; i < slice.length; i++) {
        if (slice.start[i] == '\\') {
            has_escapes = true;
            break;
        }
    }
    
    if (has_escapes) {
        // Must copy and unescape
        value.flags = JSON_STRING_FLAG_OWNED | JSON_STRING_FLAG_ESCAPED;
        value.string.owned = unescape_string(parser->pool, slice.start, slice.length);
    } else {
        // Zero-copy: just store view
        value.flags = JSON_STRING_FLAG_VIEW;
        value.string.view.data = slice.start;
        value.string.view.len = slice.length;
    }
    
    advance(parser);
    return value;
}

// Unescape string (only called for strings with escapes)
static char *unescape_string(mem_pool_t *pool, const char *src, size_t len) {
    // Worst case: same length (escapes only make it shorter)
    char *dst = pool_alloc(pool, len + 1);
    if (!dst) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (src[i] == '\\' && i + 1 < len) {
            i++;
            switch (src[i]) {
                case '"':  dst[j++] = '"';  break;
                case '\\': dst[j++] = '\\'; break;
                case '/':  dst[j++] = '/';  break;
                case 'b':  dst[j++] = '\b'; break;
                case 'f':  dst[j++] = '\f'; break;
                case 'n':  dst[j++] = '\n'; break;
                case 'r':  dst[j++] = '\r'; break;
                case 't':  dst[j++] = '\t'; break;
                case 'u':
                    // Handle Unicode escape (\uXXXX)
                    if (i + 4 < len) {
                        // TODO: Implement Unicode handling
                        i += 4;
                    }
                    break;
                default:
                    dst[j++] = src[i];
            }
        } else {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
    
    return dst;
}
```

### 6.4 Expected Performance Impact

| String Type | Current | Zero-Copy | Improvement |
|-------------|---------|-----------|-------------|
| No escapes | malloc + memcpy | No allocation | 60-80% faster |
| With escapes | malloc + unescape | pool_alloc + unescape | 20-30% faster |
| Overall (typical JSON) | 100% | 65-75% | 25-35% faster |

---

## 7. Branch Prediction Optimization

### 7.1 Using Likely/Unlikely Hints

```c
// include/compiler_hints.h

#ifndef COMPILER_HINTS_H
#define COMPILER_HINTS_H

#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define ALWAYS_INLINE __attribute__((always_inline)) inline
    #define NOINLINE __attribute__((noinline))
    #define HOT __attribute__((hot))
    #define COLD __attribute__((cold))
    #define PREFETCH(addr) __builtin_prefetch(addr)
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
    #define ALWAYS_INLINE inline
    #define NOINLINE
    #define HOT
    #define COLD
    #define PREFETCH(addr)
#endif

#endif // COMPILER_HINTS_H
```

### 7.2 Applied to Parser

```c
#include "compiler_hints.h"

// Hot path: most common token types first
HOT json_value_t parse_value(parser_t *parser) {
    token_type_t type = parser->current_token.type;
    
    // Order by frequency (strings and numbers are most common)
    if (LIKELY(type == TOKEN_STRING)) {
        return parse_string(parser);
    }
    if (LIKELY(type == TOKEN_NUMBER)) {
        return parse_number(parser);
    }
    if (LIKELY(type == TOKEN_LBRACE)) {
        return parse_object(parser);
    }
    if (LIKELY(type == TOKEN_LBRACKET)) {
        return parse_array(parser);
    }
    
    // Less common cases
    if (type == TOKEN_TRUE || type == TOKEN_FALSE) {
        return parse_boolean(parser);
    }
    if (type == TOKEN_NULL) {
        return parse_null(parser);
    }
    
    // Error case - cold path
    if (UNLIKELY(true)) {
        parser_error(parser, "Unexpected token");
        return json_value_init(JSON_NULL);
    }
}

// Hot path in lexer
HOT token_t tokenize(lexer_t *lexer) {
    skip_whitespace(lexer);
    
    if (UNLIKELY(*lexer->current == '\0')) {
        // EOF - cold path
        return (token_t){.type = TOKEN_EOF};
    }
    
    char c = *lexer->current;
    
    // Most common: strings
    if (LIKELY(c == '"')) {
        return tokenize_string(lexer);
    }
    
    // Second most common: structural characters
    if (LIKELY(c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',')) {
        token_t token = {.type = char_to_token_type(c)};
        lexer->current++;
        lexer->column++;
        return token;
    }
    
    // Numbers (including negative)
    if (LIKELY((c >= '0' && c <= '9') || c == '-')) {
        return tokenize_number(lexer);
    }
    
    // Keywords (less common)
    if (c == 't' || c == 'f' || c == 'n') {
        return tokenize_keyword(lexer);
    }
    
    // Error - very cold path
    return (token_t){.type = TOKEN_ERROR};
}
```

### 7.3 Expected Performance Impact

Branch prediction optimization typically yields 5-15% improvement on modern CPUs with good branch predictors, more on older CPUs or when parsing diverse JSON structures.

---

## 8. Cache-Friendly Data Structures

### 8.1 Current Issues

The current `json_value_t` is 40+ bytes and uses indirection:

```c
// Current: Union with pointers, causes cache misses
struct json_value {
    json_type_t type;      // 4 bytes
    union {                // 32 bytes (worst case)
        double number;
        char *string;      // Pointer → cache miss
        bool boolean;
        struct {
            json_value_t *items;  // Pointer → cache miss
            size_t len;
            size_t cap;
        } array;
        hash_table_t object;  // Contains pointers → cache misses
    };
};
```

### 8.2 Optimized Layout

```c
// Optimized: Smaller, cache-aligned
typedef struct json_value_opt {
    uint8_t type;           // 1 byte
    uint8_t flags;          // 1 byte
    uint16_t _pad;          // 2 bytes (alignment)
    uint32_t len;           // 4 bytes (for strings/arrays)
    union {                 // 8 bytes
        double number;
        const char *string; // Zero-copy pointer into input
        bool boolean;
        struct json_value_opt *children;  // For arrays/objects
    };
} json_value_opt_t;  // Total: 16 bytes (fits in cache line)

// For arrays/objects, children are stored contiguously:
// [json_value_opt_t header] [child1] [child2] [child3] ...
```

### 8.3 Structure of Arrays (SoA) for Large Collections

For very large arrays, consider SoA layout:

```c
// Instead of: json_value_t items[1000];
// Use:
typedef struct {
    uint8_t *types;      // types[1000]
    double *numbers;     // numbers[1000] (for numeric arrays)
    const char **strings;  // strings[1000] (for string arrays)
    // ... other types
    size_t len;
    size_t cap;
} json_array_soa_t;
```

This improves cache utilization when iterating over arrays of same-type elements.

### 8.4 Expected Performance Impact

| Optimization | Memory Reduction | Cache Hits | Overall Speed |
|--------------|------------------|------------|---------------|
| 16-byte json_value | 60% | +25% | +15-20% |
| SoA for arrays | 20-40% | +40% | +20-30% on iteration |

---

## 9. Benchmarking Strategy

### 9.1 Micro-benchmarks

Create isolated benchmarks for each component:

```c
// bench_lexer.c - Tokenization speed
void bench_skip_whitespace(benchmark_state *state) {
    const char *input = generate_whitespace(10000);
    while (state->keep_running) {
        const char *result = skip_whitespace_simd(input, input + 10000);
        benchmark_do_not_optimize(result);
    }
}

// bench_string.c - String parsing speed
void bench_parse_long_string(benchmark_state *state) {
    const char *input = generate_json_string(100000);
    while (state->keep_running) {
        lexer_t lexer = lexer_init(input);
        token_t token = tokenize_string(&lexer);
        benchmark_do_not_optimize(token);
        lexer_free(&lexer);
    }
}

// bench_object.c - Object creation speed
void bench_create_object(benchmark_state *state) {
    while (state->keep_running) {
        json_value_t obj = json_value_object(100);
        for (int i = 0; i < 100; i++) {
            char key[16];
            sprintf(key, "key%d", i);
            json_object_set(&obj, strdup(key), json_value_number(i));
        }
        json_value_free(&obj);
    }
}
```

### 9.2 A/B Testing Framework

```c
// bench_compare.c
typedef struct {
    const char *name;
    void (*func)(const char *input, size_t len);
} parser_variant_t;

parser_variant_t variants[] = {
    {"baseline", parse_baseline},
    {"with_pool", parse_with_pool},
    {"with_simd", parse_with_simd},
    {"with_lazy_hash", parse_with_lazy_hash},
    {"full_optimized", parse_full_optimized},
};

void compare_parsers(const char *input, size_t len) {
    printf("File size: %zu bytes\n\n", len);
    printf("%-20s %12s %12s %12s\n", "Variant", "Time (ms)", "MB/s", "vs Baseline");
    printf("%-20s %12s %12s %12s\n", "-------", "---------", "----", "-----------");
    
    double baseline_time = 0;
    
    for (size_t i = 0; i < sizeof(variants)/sizeof(variants[0]); i++) {
        double total = 0;
        for (int run = 0; run < 100; run++) {
            double start = get_time_us();
            variants[i].func(input, len);
            double end = get_time_us();
            total += (end - start);
        }
        
        double avg_us = total / 100;
        double avg_ms = avg_us / 1000;
        double mbps = (len / (1024.0 * 1024.0)) / (avg_ms / 1000.0);
        
        if (i == 0) baseline_time = avg_ms;
        
        double speedup = baseline_time / avg_ms;
        
        printf("%-20s %12.3f %12.2f %11.2fx\n", 
               variants[i].name, avg_ms, mbps, speedup);
    }
}
```

### 9.3 Profiling Integration

```bash
# CPU profiling with perf
perf record -g ./bench_parser data/large.json
perf report

# Cache analysis
perf stat -e cache-references,cache-misses,L1-dcache-load-misses ./bench_parser

# Flame graphs
perf record -F 99 -g ./bench_parser data/large.json
perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg
```

---

## 10. Implementation Roadmap

### Phase 1: Foundation (Week 1-2)

**Priority: Memory Pool**
- Implement `mem_pool.h` and `mem_pool.c`
- Integrate with lexer and parser
- Add pool statistics to benchmark
- Expected improvement: **25-35%**

**Deliverables:**
- [ ] Memory pool implementation
- [ ] Modified `lexer_init_with_pool()`
- [ ] Modified `parser_init_with_pool()`
- [ ] Benchmark comparison

### Phase 2: String Optimization (Week 3-4)

**Priority: SIMD String Processing**
- Implement SIMD whitespace skipping
- Implement SIMD string boundary detection
- Add ARM NEON support
- Expected improvement: **15-25%** (string-heavy workloads)

**Priority: Zero-Copy Strings**
- Implement string view type
- Modify string parsing to use views
- Handle escape sequences separately
- Expected improvement: **10-20%**

**Deliverables:**
- [ ] `simd_utils.h` and implementation
- [ ] Zero-copy string types
- [ ] Benchmark comparison for `long_strings.json`

### Phase 3: Object Optimization (Week 5-6)

**Priority: Lazy Hash Table**
- Implement `lazy_object.h`
- Integrate with parser
- Add linear search for small objects
- Expected improvement: **10-20%**

**Priority: String Interning**
- Implement intern table
- Use for object keys
- Optional: intern string values
- Expected improvement: **5-15%** (repeated keys)

**Deliverables:**
- [ ] Lazy object implementation
- [ ] String interning
- [ ] Benchmark comparison for `large_object.json`

### Phase 4: Fine-Tuning (Week 7-8)

**Priority: Branch Prediction**
- Add likely/unlikely hints
- Reorder switch cases by frequency
- Expected improvement: **5-10%**

**Priority: Cache Optimization**
- Reduce json_value_t size
- Improve memory layout
- Expected improvement: **5-15%**

**Deliverables:**
- [ ] Compiler hints throughout codebase
- [ ] Optimized data structures
- [ ] Final benchmark suite

### Target Performance

| File | Current | Target | V8 |
|------|---------|--------|-----|
| large.json | 398 MB/s | 800+ MB/s | 733 MB/s |
| xlarge.json | 381 MB/s | 500+ MB/s | 339 MB/s |
| long_strings.json | 911 MB/s | 2000+ MB/s | 2299 MB/s |
| large_object.json | 257 MB/s | 400+ MB/s | 295 MB/s |

---

## Appendix A: Quick Reference - Compiler Flags

```makefile
# Release build with all optimizations
CFLAGS_RELEASE = -O3 \
    -march=native \
    -flto \
    -ffast-math \
    -funroll-loops \
    -fomit-frame-pointer \
    -DNDEBUG

# SIMD-specific
CFLAGS_SIMD_X86 = -msse4.2 -mavx2
CFLAGS_SIMD_ARM = -march=armv8-a+simd

# Profile-guided optimization
# Step 1: Build with instrumentation
CFLAGS_PGO_GEN = -fprofile-generate

# Step 2: Run representative workload
# ./bench_parser data/*.json

# Step 3: Build with profile data
CFLAGS_PGO_USE = -fprofile-use
```

---

## Appendix B: Useful Profiling Commands

```bash
# Memory profiling with Valgrind
valgrind --tool=massif ./bench_parser data/large.json
ms_print massif.out.*

# Cachegrind analysis
valgrind --tool=cachegrind ./bench_parser data/large.json
cg_annotate cachegrind.out.*

# System-level performance
perf stat -d ./bench_parser data/large.json

# Instruction-level analysis
perf annotate -s parse_value
```

---

## Appendix C: Related Projects for Reference

1. **simdjson** - SIMD-accelerated JSON parser (C++)
   - https://github.com/simdjson/simdjson
   - Study their SIMD techniques for string scanning

2. **rapidjson** - Fast JSON parser (C++)
   - https://github.com/Tencent/rapidjson
   - Study their memory allocation strategies

3. **yyjson** - High-performance JSON library (C)
   - https://github.com/ibireme/yyjson
   - Study their data structure design

4. **cJSON** - Lightweight JSON parser (C)
   - https://github.com/DaveGamble/cJSON
   - Study their simplicity vs. performance tradeoffs
