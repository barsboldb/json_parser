# Benchmark Results

## Environment
- **Date**: $(date)
- **OS**: $(uname -s)
- **Architecture**: $(uname -m)
- **Node.js Version**: $(node --version)
- **Compiler**: $(gcc --version | head -n 1)

## Libraries Tested
1. **json_parser** (This project) - Custom C implementation
2. **Node.js JSON.parse()** - V8 native implementation

## Test Files
- simple.json (~0.06 KB)
- array.json (~0.03 KB)
- nested.json (~0.77 KB)
- complex.json (~0.87 KB)
- edge_cases.json (~0.55 KB)
- large_array.json (~193 KB)
- large_object.json (~78 KB)
- deeply_nested.json (~36 KB)
- real_world_api.json (~247 KB)

## Results

See individual CSV files:
- `json_parser_results.csv` - json_parser results
- `nodejs_results.csv` - Node.js results

## How to Read Results
- **AvgTime(ms)**: Lower is better
- **Throughput(MB/s)**: Higher is better
- **MinTime(ms)**: Best case performance
- **MaxTime(ms)**: Worst case performance

