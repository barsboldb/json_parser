# Memory Benchmark Results

## Overview

This benchmark measures memory usage patterns for JSON parsing:
- **Heap Allocations**: Memory allocated for JSON data structures
- **Peak Usage**: Maximum memory used during parsing
- **RSS (Resident Set Size)**: Total process memory
- **Memory Overhead**: Ratio of allocated memory to file size
- **Memory Leaks**: Unfreed memory after parsing

## Metrics Explained

### For C (json_parser)
- **Heap Allocated**: Total memory allocated via malloc
- **Heap Freed**: Total memory freed
- **Heap Peak**: Maximum heap usage during parsing
- **Leaked**: Memory not freed (should be 0)
- **Overhead Ratio**: Heap allocated / file size

### For JavaScript (Node.js)
- **Heap Before/After**: V8 heap usage before and after parsing
- **Heap Delta**: Additional heap memory used
- **Heap Retained**: Memory still used after GC
- **RSS Delta**: Change in process memory

## Results

See detailed CSV files:
- `memory_json_parser.csv` - C parser memory usage
- `memory_nodejs.csv` - Node.js memory usage

## How to Interpret

**Lower is Better:**
- Overhead ratio (closer to 1.0x is ideal)
- Leaked memory (should be 0)
- RSS delta

**Zero is Critical:**
- Memory leaks must be zero for production use

