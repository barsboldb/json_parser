# JSON Parser Benchmark Plan

## Objective
Compare parsing performance across major C JSON libraries and JavaScript's native parser.

## Libraries to Benchmark

### C Libraries
1. **json_parser** (This project)
   - Custom implementation
   - Educational/lightweight focus

2. **cJSON**
   - GitHub: DaveGamble/cJSON
   - Single-file, lightweight
   - Most popular C JSON library

3. **jansson**
   - GitHub: akheron/jansson
   - Mature, production-ready
   - Comprehensive feature set

4. **json-c**
   - GitHub: json-c/json-c
   - Reference implementation
   - Used in many production systems

5. **parson**
   - GitHub: kgabis/parson
   - Single-file, minimal dependencies
   - Simple API

### JavaScript
6. **Node.js JSON.parse()**
   - Native implementation (V8 engine)
   - Industry standard baseline

## Test Data Sets

### Small (< 1KB)
- simple.json (60 bytes)
- array.json (29 bytes)

### Medium (1KB - 10KB)
- nested.json (788 bytes)
- complex.json (893 bytes)
- edge_cases.json (567 bytes)

### Large (10KB - 100KB)
- large_array.json (generated)
- large_object.json (generated)

### Extra Large (100KB - 1MB)
- xl_nested.json (generated)
- real_world_api.json (generated)

## Metrics to Measure

1. **Parse Time**
   - Time to parse JSON string into data structure
   - Measured in microseconds (μs)

2. **Memory Usage**
   - Peak memory allocation during parsing
   - Measured in KB/MB

3. **Throughput**
   - MB/second processing rate

4. **Iterations**
   - Run each test 1000+ times for statistical significance
   - Calculate mean, median, min, max, std deviation

## Benchmark Implementation

### C Benchmark (`benchmarks/bench_c.c`)
```c
- Load JSON file
- Measure time with high-resolution timer (clock_gettime)
- Parse JSON
- Record metrics
- Free memory
- Repeat N times
```

### JavaScript Benchmark (`benchmarks/bench_js.js`)
```javascript
- Load JSON file
- Use performance.now() for timing
- Parse with JSON.parse()
- Record metrics
- Repeat N times
```

### Results Format
- CSV output for easy analysis
- Markdown table for README
- Charts/graphs (optional)

## Success Criteria

- All libraries successfully parse all test files
- Consistent, reproducible results
- Clear performance comparisons
- Fair testing methodology (no bias)

## Next Steps

1. Install/setup all C libraries
2. Generate large test data files
3. Implement benchmark harness
4. Run benchmarks
5. Analyze and document results
