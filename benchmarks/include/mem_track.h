#ifndef MEM_TRACK_H
#define MEM_TRACK_H

#include <stdlib.h>

// Forward declarations
extern int g_track_memory;
extern void* tracked_malloc(size_t size);
extern void tracked_free(void* ptr);
extern void* tracked_realloc(void* ptr, size_t size);
extern void* tracked_calloc(size_t nmemb, size_t size);

// Redirect all allocations to tracked versions
#define malloc(size) tracked_malloc(size)
#define free(ptr) tracked_free(ptr)
#define realloc(ptr, size) tracked_realloc(ptr, size)
#define calloc(nmemb, size) tracked_calloc(nmemb, size)

#endif // MEM_TRACK_H
