#!/bin/bash

# JSON Parser Memory Benchmark Runner

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}JSON Parser Memory Benchmark Suite${NC}"
echo -e "${BLUE}=========================================${NC}"
echo ""

# Create results directory
mkdir -p results

# 1. Compile memory benchmark
echo -e "${YELLOW}[1/2] Compiling memory benchmark...${NC}"
gcc src/bench_memory.c ../src/*.c -I../include -o bin/bench_memory -O3 -Wall
if [ $? -eq 0 ]; then
  echo -e "${GREEN}✓ Compilation successful${NC}"
else
  echo -e "${RED}✗ Compilation failed${NC}"
  exit 1
fi
echo ""

# 2. Run C memory benchmark
echo -e "${YELLOW}[2/2] Running C memory benchmark...${NC}"
./bin/bench_memory
if [ $? -eq 0 ]; then
  echo -e "${GREEN}✓ C memory benchmark complete${NC}"
else
  echo -e "${RED}✗ C memory benchmark failed${NC}"
  exit 1
fi
echo ""

# 3. Run JavaScript memory benchmark
echo -e "${YELLOW}[3/2] Running Node.js memory benchmark...${NC}"
node --expose-gc src/bench_memory_js.js
if [ $? -eq 0 ]; then
  echo -e "${GREEN}✓ Node.js memory benchmark complete${NC}"
else
  echo -e "${RED}✗ Node.js memory benchmark failed${NC}"
  exit 1
fi
echo ""

# 4. Generate comparison report
echo -e "${YELLOW}Generating memory comparison report...${NC}"

cat > results/MEMORY_RESULTS.md << 'EOF'
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

EOF

echo -e "${GREEN}✓ Memory comparison report generated${NC}"
echo ""

echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}Memory Benchmark Complete!${NC}"
echo -e "${BLUE}=========================================${NC}"
echo ""
echo -e "Results saved to: ${GREEN}benchmarks/results/${NC}"
echo ""
echo -e "${YELLOW}View results:${NC}"
echo "  - cat results/memory_json_parser.csv"
echo "  - cat results/memory_nodejs.csv"
echo "  - cat results/MEMORY_RESULTS.md"
echo ""
