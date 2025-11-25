#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include <sys/stat.h>

// Define BENCHMARK_MEMORY_TRACKING before including mem_track.h
// This will redirect all malloc/free calls to our tracking functions
#define BENCHMARK_MEMORY_TRACKING
#include "../include/mem_track.h"

// Now include parser headers - their malloc/free will be redirected
#include "../../include/parser.h"

#define ITERATIONS 100

typedef struct {
    char* filename;
    double parse_time_ms;
    double throughput_mbps;
    size_t file_size;
    mem_stats_t mem_stats;
} benchmark_result_t;

// Get current time in microseconds
double get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

// Read entire file into memory
char* read_file(const char* filepath, size_t* size) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filepath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = (char*)malloc(*size + 1);
    if (!content) {
        fprintf(stderr, "Error: Cannot allocate memory for file\n");
        fclose(file);
        return NULL;
    }

    fread(content, 1, *size, file);
    content[*size] = '\0';
    fclose(file);

    return content;
}

// Benchmark a single file
benchmark_result_t benchmark_file(const char* filepath) {
    benchmark_result_t result = {0};
    result.filename = strdup(filepath);

    size_t file_size;
    char* json_content = read_file(filepath, &file_size);
    if (!json_content) {
        return result;
    }
    result.file_size = file_size;

    // Warmup run
    mem_track_init();
    lexer_t warmup_lexer = lexer_init(json_content);
    parser_t warmup_parser = parser_init(&warmup_lexer);
    warmup_parser.current_token = next_token(&warmup_lexer);
    json_value_t warmup_value = parse(&warmup_parser);
    // Don't call json_value_free - parser_free will clean up pool-allocated memory
    (void)warmup_value; // Suppress unused warning
    lexer_free(&warmup_lexer);
    parser_free(&warmup_parser);

    // Benchmark runs
    double total_time = 0.0;
    mem_track_reset();

    for (int i = 0; i < ITERATIONS; i++) {
        double start = get_time_us();

        lexer_t lexer = lexer_init(json_content);
        parser_t parser = parser_init(&lexer);
        parser.current_token = next_token(&lexer);
        json_value_t value = parse(&parser);

        double end = get_time_us();
        total_time += (end - start);

        // Don't call json_value_free - parser_free will clean up pool-allocated memory
        (void)value; // Suppress unused warning
        lexer_free(&lexer);
        parser_free(&parser);
    }

    result.mem_stats = mem_track_stats();
    result.parse_time_ms = (total_time / ITERATIONS) / 1000.0;
    result.throughput_mbps = (file_size / (1024.0 * 1024.0)) / (result.parse_time_ms / 1000.0);

    free(json_content);
    return result;
}

// Get just the filename from path
const char* get_basename(const char* path) {
    const char* last_slash = strrchr(path, '/');
    return last_slash ? last_slash + 1 : path;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <data_directory> [output_perf.csv] [output_mem.csv]\n", argv[0]);
        return 1;
    }

    const char* data_dir = argv[1];
    const char* output_perf = argc > 2 ? argv[2] : "performance.csv";
    const char* output_mem = argc > 3 ? argv[3] : "memory.csv";

    // Open output files
    FILE* perf_file = fopen(output_perf, "w");
    FILE* mem_file = fopen(output_mem, "w");

    if (!perf_file || !mem_file) {
        fprintf(stderr, "Error: Cannot create output files\n");
        return 1;
    }

    // Write CSV headers
    fprintf(perf_file, "file,size_bytes,parse_time_ms,throughput_mbps\n");
    fprintf(mem_file, "file,malloc_count,free_count,realloc_count,calloc_count,total_allocated,total_freed,peak_usage,rss_start,rss_end,rss_delta,rss_peak,leaked\n");

    printf("JSON Parser Benchmark\n");
    printf("====================\n\n");

    // Read directory and benchmark each JSON file
    DIR* dir = opendir(data_dir);
    if (!dir) {
        fprintf(stderr, "Error: Cannot open directory %s\n", data_dir);
        return 1;
    }

    struct dirent* entry;
    int file_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Only process .json files
        size_t len = strlen(entry->d_name);
        if (len < 5 || strcmp(entry->d_name + len - 5, ".json") != 0) {
            continue;
        }

        // Build full path
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", data_dir, entry->d_name);

        printf("Benchmarking: %s\n", entry->d_name);

        benchmark_result_t result = benchmark_file(filepath);

        printf("  Parse time: %.3f ms\n", result.parse_time_ms);
        printf("  Throughput: %.2f MB/s\n", result.throughput_mbps);
        printf("  Memory: %zu allocs (%zu malloc, %zu calloc, %zu realloc), %zu frees\n",
               result.mem_stats.malloc_count + result.mem_stats.calloc_count + result.mem_stats.realloc_count,
               result.mem_stats.malloc_count,
               result.mem_stats.calloc_count,
               result.mem_stats.realloc_count,
               result.mem_stats.free_count);
        printf("  Peak heap: %zu bytes, RSS: %zu → %zu bytes (Δ%+zd, peak %zu)\n",
               result.mem_stats.peak_usage,
               result.mem_stats.rss_start,
               result.mem_stats.rss_end,
               (ssize_t)(result.mem_stats.rss_end - result.mem_stats.rss_start),
               result.mem_stats.rss_peak);

        size_t leaked = result.mem_stats.malloc_count + result.mem_stats.calloc_count - result.mem_stats.free_count;
        if (leaked > 0) {
            printf("  WARNING: %zu unfreed allocations detected!\n", leaked);
        }
        printf("\n");

        // Write to CSV files
        fprintf(perf_file, "%s,%zu,%.3f,%.2f\n",
                entry->d_name,
                result.file_size,
                result.parse_time_ms,
                result.throughput_mbps);

        size_t rss_delta = result.mem_stats.rss_end - result.mem_stats.rss_start;
        fprintf(mem_file, "%s,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu\n",
                entry->d_name,
                result.mem_stats.malloc_count,
                result.mem_stats.free_count,
                result.mem_stats.realloc_count,
                result.mem_stats.calloc_count,
                result.mem_stats.total_allocated,
                result.mem_stats.total_freed,
                result.mem_stats.peak_usage,
                result.mem_stats.rss_start,
                result.mem_stats.rss_end,
                rss_delta,
                result.mem_stats.rss_peak,
                leaked);

        free(result.filename);
        file_count++;
    }

    closedir(dir);
    fclose(perf_file);
    fclose(mem_file);

    printf("Benchmark complete! Processed %d files.\n", file_count);
    printf("Results written to:\n");
    printf("  - %s\n", output_perf);
    printf("  - %s\n", output_mem);

    return 0;
}
