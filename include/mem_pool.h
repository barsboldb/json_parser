#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define POOL_BLOCK_SIZE (1024 * 1024) // 1MB blocks
#define POOL_ALIGNMENT 8

typedef struct pool_block {
  struct pool_block *next;
  size_t size;
  size_t used;
  char data[];
} pool_block_t;

typedef struct {
  pool_block_t *current;
  pool_block_t *head;
  size_t total_allocated;
  size_t total_used;
  size_t block_count;
} mem_pool_t;

mem_pool_t *pool_create(void);

void *pool_alloc(mem_pool_t *pool, size_t size);
void *pool_alloc_aligned(mem_pool_t *pool, size_t size, size_t alignment);
void pool_reset(mem_pool_t *pool);
void pool_destroy(mem_pool_t *pool);

size_t pool_bytes_used(mem_pool_t *pool);
size_t pool_bytes_allocated(mem_pool_t *pool);

#endif
