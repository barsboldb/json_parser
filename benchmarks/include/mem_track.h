#ifndef MEM_TRACK_H
#define MEM_TRACK_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    size_t malloc_count;
    size_t free_count;
    size_t realloc_count;
    size_t calloc_count;
    size_t total_allocated;
    size_t total_freed;
    size_t current_usage;
    size_t peak_usage;
    size_t rss_start;
    size_t rss_end;
    size_t rss_peak;
} mem_stats_t;

void mem_track_init(void);
void mem_track_reset(void);
mem_stats_t mem_track_stats(void);
size_t mem_track_get_rss(void);
void mem_track_update_rss(void);

// Real function pointers (defined in mem_track.c)
extern void* (*real_malloc)(size_t);
extern void* (*real_calloc)(size_t, size_t);
extern void* (*real_realloc)(void*, size_t);
extern void (*real_free)(void*);

// Counters (defined in mem_track.c)
extern size_t mem_malloc_count;
extern size_t mem_calloc_count;
extern size_t mem_realloc_count;
extern size_t mem_free_count;

// Macro redirection - increments counter and calls real function
#ifdef BENCHMARK_MEMORY_TRACKING
#define malloc(size) (mem_malloc_count++, real_malloc(size))
#define calloc(nmemb, size) (mem_calloc_count++, real_calloc(nmemb, size))
#define realloc(ptr, size) (mem_realloc_count++, real_realloc(ptr, size))
#define free(ptr) (mem_free_count++, real_free(ptr))
#endif

#endif // MEM_TRACK_H
