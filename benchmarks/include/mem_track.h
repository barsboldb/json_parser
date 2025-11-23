#ifndef MEM_TRACK_H
#define MEM_TRACK_H

#include <stddef.h>
#include <stdint.h>

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

// Wrapper functions
void* mem_track_malloc(size_t size);
void* mem_track_calloc(size_t nmemb, size_t size);
void* mem_track_realloc(void* ptr, size_t size);
void mem_track_free(void* ptr);

// Macro redirection - this will redirect all malloc/free calls
// to our tracking functions when BENCHMARK_MEMORY_TRACKING is defined
#ifdef BENCHMARK_MEMORY_TRACKING
#define malloc(size) mem_track_malloc(size)
#define calloc(nmemb, size) mem_track_calloc(nmemb, size)
#define realloc(ptr, size) mem_track_realloc(ptr, size)
#define free(ptr) mem_track_free(ptr)
#endif

#endif // MEM_TRACK_H
