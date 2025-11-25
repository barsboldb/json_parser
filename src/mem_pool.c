#include "../include/mem_pool.h"

#ifdef BENCHMARK_MEMORY_TRACKING
#include "../benchmarks/include/mem_track.h"
#endif

#include <stdlib.h>
#include <string.h>

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

void *pool_alloc(mem_pool_t *pool, size_t size) {
  size = align_up(size, POOL_ALIGNMENT);

  if (pool->current->used + size <= pool->current->size) {
    void *ptr = pool->current->data + pool->current->used;
    pool->current->used += size;
    pool->total_used += size;
    return ptr;
  }

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

void pool_reset(mem_pool_t *pool) {
  if (!pool) return;

  pool_block_t *current = pool->head;
  while (current) {
    current->used = 0;
    current = current->next;
  }

  pool->current = pool->head;
  pool->total_used = 0;
}

void pool_destroy(mem_pool_t *pool) {
  if (!pool) return;

  pool_block_t *current = pool->head;
  while (current) {
    pool_block_t *next = current->next;
    free(current);
    current = next;
  }

  free(pool);
}

size_t pool_bytes_used(mem_pool_t *pool) {
  return pool ? pool->total_used : 0;
}

size_t pool_bytes_allocated(mem_pool_t *pool) {
  return pool ? pool->total_allocated : 0;
}
