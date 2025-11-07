#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

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

// Benchmark result structure
typedef struct {
  const char* filename;
  int iterations;
  double total_time_ms;
  double avg_time_ms;
  double min_time_ms;
  double max_time_ms;
  double throughput_mb_s;
  long file_size_bytes;
} benchmark_result_t;

// Run benchmark on a file
benchmark_result_t benchmark_file(const char* filepath, int iterations) {
  benchmark_result_t result = {0};
  result.filename = filepath;
  result.iterations = iterations;
  result.min_time_ms = 999999.0;
  result.max_time_ms = 0.0;

  // Read file once
  char* json = read_file(filepath);
  if (!json) {
    fprintf(stderr, "Failed to read file: %s\n", filepath);
    return result;
  }

  result.file_size_bytes = strlen(json);

  // Warm-up run
  {
    lexer_t lexer = lexer_init(json);
    parser_t parser = parser_init(&lexer);
    parser.current_token = next_token(&lexer);
    json_value_t value = parse(&parser);
    json_value_free(&value);
    lexer_free(&lexer);
  }

  // Benchmark iterations
  for (int i = 0; i < iterations; i++) {
    double start = get_time_ms();

    lexer_t lexer = lexer_init(json);
    parser_t parser = parser_init(&lexer);
    parser.current_token = next_token(&lexer);
    json_value_t value = parse(&parser);

    double end = get_time_ms();
    double elapsed = end - start;

    result.total_time_ms += elapsed;
    if (elapsed < result.min_time_ms) result.min_time_ms = elapsed;
    if (elapsed > result.max_time_ms) result.max_time_ms = elapsed;

    json_value_free(&value);
    lexer_free(&lexer);
  }

  result.avg_time_ms = result.total_time_ms / iterations;

  // Calculate throughput (MB/s)
  double avg_time_s = result.avg_time_ms / 1000.0;
  double file_size_mb = result.file_size_bytes / (1024.0 * 1024.0);
  result.throughput_mb_s = file_size_mb / avg_time_s;

  free(json);
  return result;
}

// Print benchmark results
void print_result(benchmark_result_t* result) {
  printf("\nFile: %s\n", result->filename);
  printf("  Size: %.2f KB\n", result->file_size_bytes / 1024.0);
  printf("  Iterations: %d\n", result->iterations);
  printf("  Total time: %.2f ms\n", result->total_time_ms);
  printf("  Avg time: %.4f ms\n", result->avg_time_ms);
  printf("  Min time: %.4f ms\n", result->min_time_ms);
  printf("  Max time: %.4f ms\n", result->max_time_ms);
  printf("  Throughput: %.2f MB/s\n", result->throughput_mb_s);
}

// Save results to CSV
void save_csv(benchmark_result_t* results, int count, const char* output_file) {
  FILE* file = fopen(output_file, "w");
  if (!file) {
    fprintf(stderr, "Failed to open output file: %s\n", output_file);
    return;
  }

  fprintf(file, "Library,Filename,Size(KB),Iterations,TotalTime(ms),AvgTime(ms),MinTime(ms),MaxTime(ms),Throughput(MB/s)\n");

  for (int i = 0; i < count; i++) {
    benchmark_result_t* r = &results[i];
    fprintf(file, "json_parser,%s,%.2f,%d,%.2f,%.4f,%.4f,%.4f,%.2f\n",
            r->filename,
            r->file_size_bytes / 1024.0,
            r->iterations,
            r->total_time_ms,
            r->avg_time_ms,
            r->min_time_ms,
            r->max_time_ms,
            r->throughput_mb_s);
  }

  fclose(file);
  printf("\n✓ Results saved to: %s\n", output_file);
}

int main() {
  printf("===========================================\n");
  printf("JSON Parser Benchmark - json_parser\n");
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
  benchmark_result_t results[num_files];

  // Determine iterations based on file size
  int iterations_map[] = {
    10000,  // simple.json (< 1KB)
    10000,  // array.json (< 1KB)
    5000,   // nested.json (< 1KB)
    5000,   // complex.json (< 1KB)
    5000,   // edge_cases.json (< 1KB)
    500,    // large_array.json (~200KB)
    500,    // large_object.json (~80KB)
    1000,   // deeply_nested.json (~40KB)
    500     // real_world_api.json (~250KB)
  };

  for (int i = 0; i < num_files; i++) {
    printf("\nBenchmarking: %s\n", test_files[i]);
    results[i] = benchmark_file(test_files[i], iterations_map[i]);
    print_result(&results[i]);
  }

  printf("\n===========================================\n");
  printf("Benchmark Complete!\n");
  printf("===========================================\n");

  save_csv(results, num_files, "results/json_parser_results.csv");

  return 0;
}
