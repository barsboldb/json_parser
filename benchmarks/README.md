# JSON Parser Benchmark Suite

Comprehensive benchmarking system for the JSON parser with performance and memory analysis, historical tracking, and visualization.

## Quick Start

```bash
# Generate test data (first time only)
make benchmark-data

# Run benchmarks on current code
make benchmark

# View results in browser
make benchmark-view

# Collect benchmarks for all historical commits
make benchmark-history
```

## Features

### 📊 Comprehensive Test Data

15 test files covering diverse scenarios:

**Standard Sizes:**
- `small.json` (~1KB) - Basic JSON object
- `medium.json` (~270KB) - 500 user records
- `large.json` (~13MB) - 5,000 items with nested data
- `xlarge.json` (~38MB) - 30,000 records

**Structure Types:**
- `large_array.json` (~1.2MB) - 10,000 item array
- `large_object.json` (~582KB) - 5,000 key object
- `deeply_nested.json` (~3KB) - 20 levels deep nesting

**Edge Cases:**
- `unicode.json` - Unicode, emoji, special characters
- `numeric_edges.json` - Numeric edge cases (exponentials, extremes)
- `empty_structures.json` - Empty objects/arrays
- `long_strings.json` (~1.2MB) - Very long strings (up to 1MB)

**Real-World:**
- `github_api.json` - GitHub API response format
- `package_json.json` - npm package.json format
- `config.json` - Application configuration format
- `geojson.json` - GeoJSON format with 100 features

### 🎯 Dual Benchmarking

**C Parser (Your Implementation):**
- Performance: throughput (MB/s), parse time (ms)
- Memory: allocation counts, peak usage, leak detection

**Node.js JSON.parse() (Baseline):**
- Same metrics for direct comparison
- Uses V8's native JSON parser

### 📈 Metrics Tracked

**Performance:**
- Parse time (milliseconds)
- Throughput (MB/s)
- File size correlation

**Memory:**
- malloc/free counts
- realloc usage
- Peak memory usage
- Memory leak detection (malloc - free ≠ 0)

### 🕰️ Historical Tracking

The system automatically tracks results across commits:

**Naming Convention:**
- Clean working tree: Uses commit hash (e.g., `a1b2c3d`)
- Dirty working tree: Uses timestamp (e.g., `20251121_190950`)

**Storage Structure:**
```
benchmarks/results/
├── history/
│   ├── a1b2c3d/              # Results for commit a1b2c3d
│   │   ├── metadata.json
│   │   ├── performance.csv
│   │   ├── memory.csv
│   │   ├── nodejs_performance.csv
│   │   └── nodejs_memory.csv
│   ├── 20251121_190950/      # Results for uncommitted changes
│   └── ...
├── latest -> history/20251121_190950/
└── index.json                # Index of all results
```

### 📊 Interactive Visualization

Beautiful web-based dashboard with multiple views:

**Overview Tab:**
- Latest performance statistics
- Throughput and parse time charts
- Quick summary cards

**vs Node.js Tab:**
- Side-by-side comparison charts
- Speedup factor visualization
- Performance differential analysis

**Historical Progression Tab:**
- Performance trends over time
- Per-file or average metrics
- Optimization journey visualization

**Memory Analysis Tab:**
- Allocation patterns
- Peak usage trends
- Memory leak detection and warnings

**Commit Timeline Tab:**
- Chronological list of benchmark runs
- Commit metadata and timestamps
- Quick navigation to specific runs

## Commands

### Basic Operations

```bash
# Generate test data files
make benchmark-data

# Run full benchmark suite (C + Node.js)
make benchmark

# View results in browser (starts local server on :8080)
make benchmark-view
```

### Historical Analysis

```bash
# Collect benchmarks for ALL commits
make benchmark-history

# Collect benchmarks starting from specific commit
cd benchmarks && ./scripts/collect_history.sh --start-from abc1234

# Collect benchmarks for last 10 commits only
cd benchmarks && ./scripts/collect_history.sh --limit 10

# Re-run benchmarks even if they exist
cd benchmarks && ./scripts/collect_history.sh --no-skip
```

### Manual Operations

```bash
# Run benchmark for specific commit
cd benchmarks && ./scripts/run_benchmark.sh --commit abc1234

# Force timestamp naming even with clean working tree
cd benchmarks && ./scripts/run_benchmark.sh --force-timestamp

# Generate test data manually
node benchmarks/src/generate_test_data.js

# Update index after manual changes
node benchmarks/scripts/update_index.js benchmarks/results
```

## Directory Structure

```
benchmarks/
├── src/
│   ├── bench_parser.c         # C benchmark program
│   ├── mem_track.c            # Memory tracking implementation
│   ├── bench_nodejs.js        # Node.js benchmark program
│   └── generate_test_data.js  # Test data generator
├── include/
│   └── mem_track.h            # Memory tracking header
├── scripts/
│   ├── run_benchmark.sh       # Main benchmark runner
│   ├── collect_history.sh     # Historical data collector
│   └── update_index.js        # Index updater
├── data/                      # Generated test JSON files
│   └── *.json
├── results/
│   ├── history/               # All benchmark results
│   ├── latest/                # Symlink to most recent
│   └── index.json             # Index of all results
├── bin/                       # Compiled benchmark binaries
├── visualize.html             # Interactive dashboard
└── README.md                  # This file
```

## Understanding Results

### Performance Metrics

**Throughput (MB/s):** Higher is better
- How many megabytes of JSON can be parsed per second
- Varies by file structure complexity
- Arrays/objects with many keys are slower than simple structures

**Parse Time (ms):** Lower is better
- Total time to parse the entire file
- Includes lexing and parsing
- Scales with file size and complexity

### Memory Metrics

**Malloc Count:** Number of memory allocations
- More allocations = more overhead
- Fewer is generally better

**Peak Usage:** Maximum memory used during parsing
- Important for embedded systems
- Should scale reasonably with file size

**Leaked Allocations:** malloc_count - free_count
- Should be 0 for all files
- Non-zero indicates memory leaks

### Comparison with Node.js

**Speedup Factor > 1:** Your parser is faster
**Speedup Factor < 1:** Node.js is faster

Node.js uses V8's highly optimized JSON parser, so:
- Competitive performance (~0.5-2x) is excellent
- Beating V8 on specific workloads is impressive
- Different characteristics (memory, startup) matter too

## Tips

### Accurate Benchmarks

1. **Close unnecessary applications** - Reduce system noise
2. **Use clean working tree** - Commit changes before benchmarking
3. **Warm up the system** - Run benchmark twice, use second result
4. **Consistent environment** - Same machine, same Node.js version

### Historical Analysis

When collecting historical benchmarks:
- Expect some commits to fail (API changes, build errors)
- Use `--start-from` to skip old commits
- Use `--limit` to test on recent commits only
- The script automatically handles failures and continues

### Visualization

The visualization auto-updates when you:
1. Run new benchmarks
2. View it in browser
3. Refresh the page

No need to rebuild or restart the server.

## Troubleshooting

**"No benchmark results found"**
- Run `make benchmark` first

**"Cannot open directory"**
- Run `make benchmark-data` to generate test files

**Build errors during historical collection**
- Normal for old commits with API changes
- Script continues with other commits
- Check the log for specific errors

**Memory tracking shows 0**
- Memory tracking is passive in current implementation
- The framework is ready for active tracking
- Allocation counts require library-wide malloc redefinition

**Visualization not loading**
- Check if results/index.json exists
- Ensure browser allows local file access
- Try different browser

## Future Enhancements

- [ ] Active memory tracking (require malloc wrapper in library)
- [ ] Benchmark different optimization flags
- [ ] Compare against other JSON parsers (jsmn, cJSON, rapidjson)
- [ ] Add worst-case complexity analysis
- [ ] Streaming parser benchmarks
- [ ] Memory profiling with Valgrind integration
- [ ] CPU profiling integration

## License

Same as the main project.
