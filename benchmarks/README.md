# JSON Parser Benchmarks

Comprehensive performance and memory benchmarking suite for the json_parser project.

## Quick Start

### Run All Benchmarks

```bash
# Performance benchmarks (parse time + throughput)
./run_benchmarks.sh

# Memory benchmarks
./run_memory_benchmarks.sh
```

## Latest Results

### Performance Summary (After String Slice Optimization)

| File | Size | Parse Time | Throughput | vs Node.js |
|------|------|------------|------------|------------|
| simple.json | 0.06 KB | 0.0003 ms | 213 MB/s | 🟢 **Faster!** |
| array.json | 0.03 KB | 0.0002 ms | 149 MB/s | 🟢 **Tied** |
| nested.json | 0.77 KB | 0.0026 ms | 287 MB/s | 82% of V8 |
| complex.json | 0.87 KB | 0.0026 ms | 321 MB/s | 89% of V8 |
| large_array.json | 192.73 KB | 0.5804 ms | 324 MB/s | 88% of V8 |
| **deeply_nested.json** | 36.04 KB | 0.0461 ms | **765 MB/s** | 81% of V8 |
| real_world_api.json | 247.09 KB | 0.5829 ms | 414 MB/s | 70% of V8 |

### String Slice Optimization Impact

**Performance**: 🚀 **1.9x faster** on average (up to 2.0x)
**Memory**: 💾 **95%+ reduction** in lexeme allocations
**Peak Throughput**: 📈 **765 MB/s** (+56% from 489 MB/s)

👉 See [OPTIMIZATION_IMPACT.md](OPTIMIZATION_IMPACT.md) for detailed analysis

## Directory Structure

```
benchmarks/
├── README.md                    # This file
├── run_benchmarks.sh            # Performance benchmark runner
├── run_memory_benchmarks.sh     # Memory benchmark runner
├── OPTIMIZATION_IMPACT.md       # String slice optimization analysis
│
├── src/                         # Benchmark source files
│   ├── bench_our_parser.c       # C parser performance benchmark
│   ├── bench_memory.c           # C parser memory benchmark
│   ├── bench_javascript.js      # Node.js performance benchmark
│   ├── bench_memory_js.js       # Node.js memory benchmark
│   └── generate_test_data.js    # Test data generator
│
├── bin/                         # Compiled binaries
│   ├── bench_our_parser
│   └── bench_memory
│
├── data/                        # Generated test data
│   ├── large_array.json         # ~193 KB array
│   ├── large_object.json        # ~78 KB object
│   ├── deeply_nested.json       # ~36 KB nested structure
│   └── real_world_api.json      # ~247 KB realistic API response
│
├── results/                     # Latest benchmark results
│   ├── json_parser_results.csv  # C parser performance
│   ├── nodejs_results.csv       # Node.js performance
│   ├── memory_json_parser.csv   # C parser memory
│   └── memory_nodejs.csv        # Node.js memory
│
├── results-archive/             # Historical results
│   └── before-string-slice/     # Pre-optimization baseline
│
├── archives/                    # Old documentation
│   ├── benchmark_plan.md
│   ├── BENCHMARK_RESULTS.md
│   └── MEMORY_ANALYSIS.md
│
└── libs/                        # Third-party libraries (future)
```

## Test Data

### Small Files (< 1 KB)
- `../samples/simple.json` - 0.06 KB - Basic object
- `../samples/array.json` - 0.03 KB - Simple array
- `../samples/nested.json` - 0.77 KB - Nested structure
- `../samples/complex.json` - 0.87 KB - Mixed types
- `../samples/edge_cases.json` - 0.55 KB - Edge cases

### Large Files (Generated)
- `data/large_array.json` - 192.73 KB - 10,000 element array
- `data/large_object.json` - 77.87 KB - Large object with many keys
- `data/deeply_nested.json` - 36.04 KB - 100 levels of nesting
- `data/real_world_api.json` - 247.09 KB - Realistic API response

## Benchmark Metrics

### Performance Benchmarks

Measures parsing speed and throughput:

- **Total Time**: Total time for all iterations
- **Average Time**: Mean parse time per iteration
- **Min/Max Time**: Best and worst case times
- **Throughput**: MB/s processing rate

### Memory Benchmarks

Measures memory allocation and usage:

**C Parser**:
- Heap allocations/frees (with instrumentation)
- Peak heap usage
- Memory leaks
- Process RSS (Resident Set Size)

**Node.js**:
- V8 heap usage before/after parsing
- Heap delta (additional memory used)
- Retained memory after GC
- Process RSS delta

## Benchmark Configuration

### Iterations

Benchmarks run multiple iterations for statistical accuracy:

| File Size | Iterations |
|-----------|------------|
| < 1 KB | 5,000 - 10,000 |
| 1-100 KB | 1,000 |
| > 100 KB | 500 |

### Compiler Flags

```bash
gcc -O3 -Wall -I../include
```

- `-O3`: Maximum optimization
- `-Wall`: All warnings
- `-I../include`: Include project headers

## Results Files

### CSV Format

All results are saved in CSV format for easy analysis:

```csv
Library,Filename,Size(KB),Iterations,TotalTime(ms),AvgTime(ms),MinTime(ms),MaxTime(ms),Throughput(MB/s)
json_parser,simple.json,0.06,10000,2.68,0.0003,0.0000,0.0081,213.15
```

### Memory CSV Format

```csv
Library,Filename,FileSize(KB),ParseTime(ms),HeapAlloc(KB),HeapFreed(KB),HeapPeak(KB),HeapLeaked(KB),OverheadRatio,RSSBefore(KB),RSSAfter(KB),RSSDelta(KB)
```

## Historical Benchmarks

### Before String Slice Optimization (Baseline)

See `results-archive/before-string-slice/` for pre-optimization results.

**Key metrics**:
- Average parse time: 2x slower than current
- Heap allocations: 50-500 per file
- Memory overhead: 3-5x file size
- Peak throughput: 489 MB/s

### After String Slice Optimization (Current)

See `results/` for latest results.

**Key improvements**:
- **1.9x faster** on average
- **0 heap allocations** for lexemes
- Memory overhead: Minimal
- Peak throughput: **765 MB/s**

## Environment

**OS**: macOS (Darwin ARM64)
**Compiler**: GCC with -O3
**Node.js**: v20.11.0
**V8**: 11.3.244.8

## Adding New Benchmarks

### 1. Add Test File

Place in `data/` directory:
```bash
cp my_test.json benchmarks/data/
```

### 2. Update Benchmark Scripts

Add to `src/bench_our_parser.c`:
```c
benchmark_file("data/my_test.json", 1000);
```

Add to `src/bench_javascript.js`:
```javascript
testFiles.push('data/my_test.json');
iterationsMap.push(1000);
```

### 3. Run Benchmarks

```bash
./run_benchmarks.sh
```

## Analyzing Results

### View CSV Results

```bash
# Performance results
cat results/json_parser_results.csv
cat results/nodejs_results.csv

# Memory results
cat results/memory_json_parser.csv
cat results/memory_nodejs.csv
```

### Compare with Previous Results

```bash
# Compare current vs baseline
diff results/json_parser_results.csv results-archive/before-string-slice/json_parser_results.csv
```

### Generate Charts (Optional)

Use any spreadsheet software or Python:

```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('results/json_parser_results.csv')
df.plot(x='Filename', y='Throughput(MB/s)', kind='bar')
plt.show()
```

## Future Benchmarks

### Planned Comparisons

- **cJSON** - Popular lightweight C library
- **jansson** - Production-ready C library
- **yyjson** - High-performance C library
- **simdjson** - SIMD-accelerated parser

### Planned Metrics

- Instruction-level profiling (Instruments/perf)
- Cache miss rates
- Branch prediction accuracy
- Memory allocation patterns

## Contributing

To add new benchmarks or improve existing ones:

1. Follow the existing structure in `src/`
2. Update both C and JavaScript benchmarks
3. Ensure CSV output format is consistent
4. Document any new metrics in this README
5. Run all benchmarks before committing

## License

Same as parent project (MIT).

---

*Last updated: 2024-11-18*
*Optimization: Zero-Copy String Slices (commit 91f097f)*
