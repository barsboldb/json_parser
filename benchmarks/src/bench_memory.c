#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

// Memory tracking structure
typedef struct {
  size_t allocated_bytes;
  size_t freed_bytes;
  size_t peak_usage_bytes;
  size_t current_usage_bytes;
  int allocation_count;
  int free_count;
} memory_stats_t;

memory_stats_t g_mem_stats = {0};
int g_track_memory = 0;

// Override malloc to track allocations
void* tracked_malloc(size_t size) {
  void* ptr = malloc(size + sizeof(size_t));
  if (ptr && g_track_memory) {
    *(size_t*)ptr = size;
    g_mem_stats.allocated_bytes += size;
    g_mem_stats.current_usage_bytes += size;
    g_mem_stats.allocation_count++;

    if (g_mem_stats.current_usage_bytes > g_mem_stats.peak_usage_bytes) {
      g_mem_stats.peak_usage_bytes = g_mem_stats.current_usage_bytes;
    }

    return (char*)ptr + sizeof(size_t);
  }
  return ptr ? (char*)ptr + sizeof(size_t) : NULL;
}

// Override free to track deallocations
void tracked_free(void* ptr) {
  if (ptr && g_track_memory) {
    void* real_ptr = (char*)ptr - sizeof(size_t);
    size_t size = *(size_t*)real_ptr;
    g_mem_stats.freed_bytes += size;
    g_mem_stats.current_usage_bytes -= size;
    g_mem_stats.free_count++;
    free(real_ptr);
  } else if (ptr) {
    free((char*)ptr - sizeof(size_t));
  }
}

// Override realloc to track reallocations
void* tracked_realloc(void* ptr, size_t size) {
  if (!ptr) {
    return tracked_malloc(size);
  }

  if (g_track_memory) {
    // Get old size
    void* real_ptr = (char*)ptr - sizeof(size_t);
    size_t old_size = *(size_t*)real_ptr;

    // Allocate new memory
    void* new_ptr = realloc(real_ptr, size + sizeof(size_t));
    if (!new_ptr) {
      return NULL;
    }

    // Update stats
    g_mem_stats.freed_bytes += old_size;
    g_mem_stats.current_usage_bytes -= old_size;
    g_mem_stats.allocated_bytes += size;
    g_mem_stats.current_usage_bytes += size;
    g_mem_stats.allocation_count++;

    if (g_mem_stats.current_usage_bytes > g_mem_stats.peak_usage_bytes) {
      g_mem_stats.peak_usage_bytes = g_mem_stats.current_usage_bytes;
    }

    *(size_t*)new_ptr = size;
    return (char*)new_ptr + sizeof(size_t);
  }

  void* real_ptr = (char*)ptr - sizeof(size_t);
  void* new_ptr = realloc(real_ptr, size + sizeof(size_t));
  return new_ptr ? (char*)new_ptr + sizeof(size_t) : NULL;
}

// Override calloc to track allocations
void* tracked_calloc(size_t nmemb, size_t size) {
  size_t total_size = nmemb * size;
  void* ptr = tracked_malloc(total_size);
  if (ptr) {
    memset(ptr, 0, total_size);
  }
  return ptr;
}

// Get process memory usage (RSS)
long get_rss_kb() {
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_maxrss / 1024; // Convert to KB (on macOS it's in bytes)
}

// High-resolution timer
double get_time_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

// Read file contents
char* read_file(const char* filepath) {
  FILE* file = fopen(filepath, "r");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", filepath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* content = malloc(length + 1);
  if (!content) {
    fclose(file);
    return NULL;
  }

  fread(content, 1, length, file);
  content[length] = '\0';
  fclose(file);

  return content;
}

// Memory benchmark result structure
typedef struct {
  const char* filename;
  long file_size_bytes;

  // Memory metrics
  size_t heap_allocated_kb;
  size_t heap_freed_kb;
  size_t heap_peak_kb;
  size_t heap_leaked_kb;
  int allocation_count;
  int free_count;

  // Process memory
  long rss_before_kb;
  long rss_after_kb;
  long rss_delta_kb;

  // Parse time
  double parse_time_ms;
} memory_benchmark_result_t;

// Run memory benchmark on a file
memory_benchmark_result_t benchmark_memory(const char* filepath) {
  memory_benchmark_result_t result = {0};
  result.filename = filepath;

  // Read file
  char* json = read_file(filepath);
  if (!json) {
    fprintf(stderr, "Failed to read file: %s\n", filepath);
    return result;
  }

  result.file_size_bytes = strlen(json);

  // Get initial RSS
  result.rss_before_kb = get_rss_kb();

  // Reset memory tracking
  g_mem_stats = (memory_stats_t){0};
  g_track_memory = 1;

  // Parse with timing
  double start = get_time_ms();

  lexer_t lexer = lexer_init(json);
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);
  json_value_t value = parse(&parser);

  double end = get_time_ms();
  result.parse_time_ms = end - start;

  // Capture memory stats before cleanup
  result.heap_allocated_kb = g_mem_stats.allocated_bytes / 1024;
  result.heap_peak_kb = g_mem_stats.peak_usage_bytes / 1024;
  result.allocation_count = g_mem_stats.allocation_count;

  // Clean up
  json_value_free(&value);
  lexer_free(&lexer);

  // Capture stats after cleanup
  result.heap_freed_kb = g_mem_stats.freed_bytes / 1024;
  result.heap_leaked_kb = (g_mem_stats.allocated_bytes - g_mem_stats.freed_bytes) / 1024;
  result.free_count = g_mem_stats.free_count;

  g_track_memory = 0;

  // Get final RSS
  result.rss_after_kb = get_rss_kb();
  result.rss_delta_kb = result.rss_after_kb - result.rss_before_kb;

  free(json);
  return result;
}

// Print memory benchmark result
void print_memory_result(memory_benchmark_result_t* result) {
  printf("\nFile: %s\n", result->filename);
  printf("  File size: %.2f KB\n", result->file_size_bytes / 1024.0);
  printf("  Parse time: %.4f ms\n", result->parse_time_ms);
  printf("\n  Heap Memory:\n");
  printf("    Allocated: %zu KB (%d allocations)\n",
         result->heap_allocated_kb, result->allocation_count);
  printf("    Freed: %zu KB (%d frees)\n",
         result->heap_freed_kb, result->free_count);
  printf("    Peak usage: %zu KB\n", result->heap_peak_kb);
  printf("    Leaked: %zu KB\n", result->heap_leaked_kb);
  printf("    Overhead ratio: %.2fx (allocated / file size)\n",
         (double)result->heap_allocated_kb / (result->file_size_bytes / 1024.0));
  printf("\n  Process Memory (RSS):\n");
  printf("    Before: %ld KB\n", result->rss_before_kb);
  printf("    After: %ld KB\n", result->rss_after_kb);
  printf("    Delta: %ld KB\n", result->rss_delta_kb);
}

// Save results to CSV
void save_memory_csv(memory_benchmark_result_t* results, int count, const char* output_file) {
  FILE* file = fopen(output_file, "w");
  if (!file) {
    fprintf(stderr, "Failed to open output file: %s\n", output_file);
    return;
  }

  fprintf(file, "Library,Filename,FileSize(KB),ParseTime(ms),HeapAllocated(KB),HeapFreed(KB),HeapPeak(KB),HeapLeaked(KB),AllocCount,FreeCount,OverheadRatio,RSS_Before(KB),RSS_After(KB),RSS_Delta(KB)\n");

  for (int i = 0; i < count; i++) {
    memory_benchmark_result_t* r = &results[i];
    double overhead_ratio = (double)r->heap_allocated_kb / (r->file_size_bytes / 1024.0);

    fprintf(file, "json_parser,%s,%.2f,%.4f,%zu,%zu,%zu,%zu,%d,%d,%.2f,%ld,%ld,%ld\n",
            r->filename,
            r->file_size_bytes / 1024.0,
            r->parse_time_ms,
            r->heap_allocated_kb,
            r->heap_freed_kb,
            r->heap_peak_kb,
            r->heap_leaked_kb,
            r->allocation_count,
            r->free_count,
            overhead_ratio,
            r->rss_before_kb,
            r->rss_after_kb,
            r->rss_delta_kb);
  }

  fclose(file);
  printf("\n✓ Memory results saved to: %s\n", output_file);
}

int main() {
  printf("===========================================\n");
  printf("JSON Parser Memory Benchmark - json_parser\n");
  printf("===========================================\n");

  const char* test_files[] = {
    "../samples/simple.json",
    "../samples/array.json",
    "../samples/nested.json",
    "../samples/complex.json",
    "../samples/edge_cases.json",
    "data/large_array.json",
    "data/large_object.json",
    "data/deeply_nested.json",
    "data/real_world_api.json"
  };

  int num_files = sizeof(test_files) / sizeof(test_files[0]);
  memory_benchmark_result_t results[num_files];

  for (int i = 0; i < num_files; i++) {
    printf("\nBenchmarking: %s\n", test_files[i]);
    results[i] = benchmark_memory(test_files[i]);
    print_memory_result(&results[i]);
  }

  printf("\n===========================================\n");
  printf("Memory Benchmark Complete!\n");
  printf("===========================================\n");

  save_memory_csv(results, num_files, "results/memory_json_parser.csv");

  return 0;
}
