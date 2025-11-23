// Include stdlib first to get real malloc/free
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/task.h>
#elif __linux__
#include <unistd.h>
#endif

// Save pointers to real malloc/free before they get redefined
static void* (*system_malloc)(size_t) = malloc;
static void (*system_free)(void*) = free;
static void* (*system_calloc)(size_t, size_t) = calloc;
static void* (*system_realloc)(void*, size_t) = realloc;

// Now include mem_track.h which will redefine malloc/free
#include "../include/mem_track.h"

// Hash map for tracking allocation sizes
#define MAP_SIZE 16384
typedef struct alloc_entry {
    void* ptr;
    size_t size;
    struct alloc_entry* next;
} alloc_entry_t;

static alloc_entry_t* alloc_map[MAP_SIZE] = {0};
static mem_stats_t stats = {0};

// Hash function
static size_t hash_ptr(void* ptr) {
    return ((size_t)ptr >> 3) % MAP_SIZE;
}

// Add allocation to map (use a static buffer to avoid malloc recursion)
#define ENTRY_POOL_SIZE 65536
static alloc_entry_t entry_pool[ENTRY_POOL_SIZE];
static size_t entry_pool_used = 0;

static void map_add(void* ptr, size_t size) {
    if (entry_pool_used >= ENTRY_POOL_SIZE) {
        // Pool exhausted, skip tracking this allocation
        return;
    }

    size_t idx = hash_ptr(ptr);
    alloc_entry_t* entry = &entry_pool[entry_pool_used++];
    entry->ptr = ptr;
    entry->size = size;
    entry->next = alloc_map[idx];
    alloc_map[idx] = entry;
}

// Remove and get allocation size from map
static size_t map_remove(void* ptr) {
    size_t idx = hash_ptr(ptr);
    alloc_entry_t** curr = &alloc_map[idx];

    while (*curr) {
        if ((*curr)->ptr == ptr) {
            alloc_entry_t* entry = *curr;
            size_t size = entry->size;
            *curr = entry->next;
            // Don't free entry since it's from the pool
            return size;
        }
        curr = &(*curr)->next;
    }
    return 0;
}

// Get current RSS (Resident Set Size)
size_t mem_track_get_rss(void) {
#ifdef __APPLE__
    struct mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;

    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  (task_info_t)&info, &count) == KERN_SUCCESS) {
        return info.resident_size;
    }
    return 0;
#elif __linux__
    FILE* fp = fopen("/proc/self/statm", "r");
    if (!fp) return 0;

    long rss = 0;
    if (fscanf(fp, "%*s%ld", &rss) == 1) {
        fclose(fp);
        return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);
    }
    fclose(fp);
    return 0;
#else
    return 0;
#endif
}

void mem_track_update_rss(void) {
    size_t current_rss = mem_track_get_rss();
    if (current_rss > stats.rss_peak) {
        stats.rss_peak = current_rss;
    }
}

void mem_track_init(void) {
    memset(&stats, 0, sizeof(mem_stats_t));
    memset(alloc_map, 0, sizeof(alloc_map));
    stats.rss_start = mem_track_get_rss();
    stats.rss_peak = stats.rss_start;
}

void mem_track_reset(void) {
    // Clear the map (entries are from pool, no need to free)
    for (int i = 0; i < MAP_SIZE; i++) {
        alloc_map[i] = NULL;
    }
    entry_pool_used = 0;
    mem_track_init();
}

mem_stats_t mem_track_stats(void) {
    stats.rss_end = mem_track_get_rss();
    return stats;
}

void* mem_track_malloc(size_t size) {
    void* ptr = system_malloc(size);
    if (ptr) {
        stats.malloc_count++;
        stats.total_allocated += size;
        stats.current_usage += size;
        if (stats.current_usage > stats.peak_usage) {
            stats.peak_usage = stats.current_usage;
        }
        map_add(ptr, size);
        mem_track_update_rss();
    }
    return ptr;
}

void* mem_track_calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void* ptr = system_calloc(nmemb, size);
    if (ptr) {
        stats.calloc_count++;
        stats.total_allocated += total_size;
        stats.current_usage += total_size;
        if (stats.current_usage > stats.peak_usage) {
            stats.peak_usage = stats.current_usage;
        }
        map_add(ptr, total_size);
        mem_track_update_rss();
    }
    return ptr;
}

void* mem_track_realloc(void* ptr, size_t size) {
    size_t old_size = 0;
    if (ptr) {
        old_size = map_remove(ptr);
        stats.current_usage -= old_size;
        stats.total_freed += old_size;
    }

    void* new_ptr = system_realloc(ptr, size);
    if (new_ptr) {
        stats.realloc_count++;
        stats.total_allocated += size;
        stats.current_usage += size;
        if (stats.current_usage > stats.peak_usage) {
            stats.peak_usage = stats.current_usage;
        }
        map_add(new_ptr, size);
        mem_track_update_rss();
    }
    return new_ptr;
}

void mem_track_free(void* ptr) {
    if (ptr) {
        stats.free_count++;
        size_t size = map_remove(ptr);
        stats.total_freed += size;
        stats.current_usage -= size;
        system_free(ptr);
        mem_track_update_rss();
    }
}
