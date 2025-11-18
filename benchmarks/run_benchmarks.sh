#!/bin/bash

# JSON Parser Benchmark Suite Runner

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
echo -e "${BLUE}JSON Parser Benchmark Suite${NC}"
echo -e "${BLUE}=========================================${NC}"
echo ""

# Create results directory
mkdir -p results

# 1. Compile our parser benchmark
echo -e "${YELLOW}[1/3] Compiling json_parser benchmark...${NC}"
gcc src/bench_our_parser.c ../src/*.c -I../include -o bin/bench_our_parser -O3 -Wall
if [ $? -eq 0 ]; then
  echo -e "${GREEN}✓ Compilation successful${NC}"
else
  echo -e "${RED}✗ Compilation failed${NC}"
  exit 1
fi
echo ""

# 2. Run our parser benchmark
echo -e "${YELLOW}[2/3] Running json_parser benchmark...${NC}"
./bin/bench_our_parser
if [ $? -eq 0 ]; then
  echo -e "${GREEN}✓ json_parser benchmark complete${NC}"
else
  echo -e "${RED}✗ json_parser benchmark failed${NC}"
  exit 1
fi
echo ""

# 3. Run JavaScript benchmark
echo -e "${YELLOW}[3/3] Running Node.js JSON.parse() benchmark...${NC}"
node src/bench_javascript.js
if [ $? -eq 0 ]; then
  echo -e "${GREEN}✓ Node.js benchmark complete${NC}"
else
  echo -e "${RED}✗ Node.js benchmark failed${NC}"
  exit 1
fi
echo ""

# 4. Generate comparison report
echo -e "${YELLOW}Generating comparison report...${NC}"

cat > results/README.md << 'EOF'
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

EOF

echo -e "${GREEN}✓ Comparison report generated${NC}"
echo ""

echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}Benchmark Complete!${NC}"
echo -e "${BLUE}=========================================${NC}"
echo ""
echo -e "Results saved to: ${GREEN}benchmarks/results/${NC}"
echo ""
echo -e "${YELLOW}View results:${NC}"
echo "  - cat results/json_parser_results.csv"
echo "  - cat results/nodejs_results.csv"
echo "  - cat results/README.md"
echo ""
