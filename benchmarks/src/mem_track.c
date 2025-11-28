// mem_track.c - Memory allocation tracking
// This file must be compiled WITHOUT -DBENCHMARK_MEMORY_TRACKING

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/task.h>
#elif __linux__
#include <unistd.h>
#endif

// Export real function pointers (before any macro redefinition)
void* (*real_malloc)(size_t) = malloc;
void* (*real_calloc)(size_t, size_t) = calloc;
void* (*real_realloc)(void*, size_t) = realloc;
void (*real_free)(void*) = free;

// Define mem_stats_t locally (same as in header)
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
    size_t pool_allocated;
    size_t pool_used;
} mem_stats_t;

// Export counters
size_t mem_malloc_count = 0;
size_t mem_calloc_count = 0;
size_t mem_realloc_count = 0;
size_t mem_free_count = 0;

// Internal stats
static size_t total_allocated = 0;
static size_t total_freed = 0;
static size_t current_usage = 0;
static size_t peak_usage = 0;
static size_t rss_start = 0;
static size_t rss_end = 0;
static size_t rss_peak = 0;

// Allocation size tracking table (simple hash map)
#define ALLOC_TABLE_SIZE 16384
typedef struct alloc_entry {
    void* ptr;
    size_t size;
    struct alloc_entry* next;
} alloc_entry_t;

static alloc_entry_t* alloc_table[ALLOC_TABLE_SIZE] = {0};

static size_t hash_ptr(void* ptr) {
    return ((uintptr_t)ptr >> 3) % ALLOC_TABLE_SIZE;
}

static void track_alloc(void* ptr, size_t size) {
    if (!ptr) return;

    size_t idx = hash_ptr(ptr);
    alloc_entry_t* entry = real_malloc(sizeof(alloc_entry_t));
    entry->ptr = ptr;
    entry->size = size;
    entry->next = alloc_table[idx];
    alloc_table[idx] = entry;

    total_allocated += size;
    current_usage += size;
    if (current_usage > peak_usage) {
        peak_usage = current_usage;
    }
}

static size_t untrack_alloc(void* ptr) {
    if (!ptr) return 0;

    size_t idx = hash_ptr(ptr);
    alloc_entry_t** curr = &alloc_table[idx];

    while (*curr) {
        if ((*curr)->ptr == ptr) {
            size_t size = (*curr)->size;
            alloc_entry_t* to_free = *curr;
            *curr = (*curr)->next;
            real_free(to_free);

            total_freed += size;
            current_usage -= size;
            return size;
        }
        curr = &(*curr)->next;
    }
    return 0;
}

// Get RSS (Resident Set Size) - actual memory used by process
size_t mem_track_get_rss(void) {
#ifdef __APPLE__
    struct mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) == KERN_SUCCESS) {
        return info.resident_size;
    }
    return 0;
#elif __linux__
    FILE* fp = fopen("/proc/self/statm", "r");
    if (fp) {
        long pages = 0;
        if (fscanf(fp, "%*d %ld", &pages) == 1) {
            fclose(fp);
            return pages * sysconf(_SC_PAGESIZE);
        }
        fclose(fp);
    }
    return 0;
#else
    return 0;
#endif
}

void mem_track_init(void) {
    mem_malloc_count = 0;
    mem_calloc_count = 0;
    mem_realloc_count = 0;
    mem_free_count = 0;
    total_allocated = 0;
    total_freed = 0;
    current_usage = 0;
    peak_usage = 0;
    rss_start = mem_track_get_rss();
    rss_peak = rss_start;
    rss_end = 0;

    // Clear allocation table
    for (size_t i = 0; i < ALLOC_TABLE_SIZE; i++) {
        alloc_entry_t* entry = alloc_table[i];
        while (entry) {
            alloc_entry_t* next = entry->next;
            real_free(entry);
            entry = next;
        }
        alloc_table[i] = NULL;
    }
}

void mem_track_reset(void) {
    mem_track_init();
}

void mem_track_update_rss(void) {
    size_t current_rss = mem_track_get_rss();
    if (current_rss > rss_peak) {
        rss_peak = current_rss;
    }
    rss_end = current_rss;
}

mem_stats_t mem_track_stats(void) {
    mem_track_update_rss();

    mem_stats_t stats;
    stats.malloc_count = mem_malloc_count;
    stats.free_count = mem_free_count;
    stats.realloc_count = mem_realloc_count;
    stats.calloc_count = mem_calloc_count;
    stats.total_allocated = total_allocated;
    stats.total_freed = total_freed;
    stats.current_usage = current_usage;
    stats.peak_usage = peak_usage;
    stats.rss_start = rss_start;
    stats.rss_end = rss_end;
    stats.rss_peak = rss_peak;
    stats.pool_allocated = 0;
    stats.pool_used = 0;
    return stats;
}

// Wrapper functions that track sizes
void* tracked_malloc(size_t size) {
    mem_malloc_count++;
    void* ptr = real_malloc(size);
    track_alloc(ptr, size);
    return ptr;
}

void* tracked_calloc(size_t nmemb, size_t size) {
    mem_calloc_count++;
    void* ptr = real_calloc(nmemb, size);
    track_alloc(ptr, nmemb * size);
    return ptr;
}

void* tracked_realloc(void* ptr, size_t size) {
    mem_realloc_count++;
    untrack_alloc(ptr);
    void* new_ptr = real_realloc(ptr, size);
    track_alloc(new_ptr, size);
    return new_ptr;
}

void tracked_free(void* ptr) {
    mem_free_count++;
    untrack_alloc(ptr);
    real_free(ptr);
}
