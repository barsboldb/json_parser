#!/bin/bash
# Comprehensive benchmark runner with git metadata capture
# Usage: ./scripts/run_benchmark.sh [--perf-only | --memory-only]

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Configuration
BENCHMARK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_ROOT="$(dirname "$BENCHMARK_DIR")"
RESULTS_DIR="$BENCHMARK_DIR/results"
HISTORY_DIR="$RESULTS_DIR/history"
SCRIPTS_DIR="$BENCHMARK_DIR/scripts"

# Parse arguments
RUN_PERF=true
RUN_MEMORY=true

for arg in "$@"; do
  case $arg in
    --perf-only)
      RUN_MEMORY=false
      ;;
    --memory-only)
      RUN_PERF=false
      ;;
    *)
      echo -e "${RED}Unknown argument: $arg${NC}"
      echo "Usage: $0 [--perf-only | --memory-only]"
      exit 1
      ;;
  esac
done

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}JSON Parser Benchmark Suite${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Capture git metadata
echo -e "${CYAN}📊 Capturing git metadata...${NC}"
cd "$PROJECT_ROOT"

GIT_COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
GIT_COMMIT_FULL=$(git rev-parse HEAD 2>/dev/null || echo "unknown")
GIT_MESSAGE=$(git log -1 --pretty=%B 2>/dev/null | head -1 || echo "No commit message")
GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
GIT_DIRTY=false
if [[ -n $(git status --porcelain 2>/dev/null) ]]; then
  GIT_DIRTY=true
fi

# Use commit timestamp for clean commits, current time for dirty ones
if [ "$GIT_DIRTY" = true ]; then
  TIMESTAMP=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
  TIMESTAMP_DIR=$(date -u +"%Y-%m-%d_%H%M%S")
else
  # Get commit timestamp (ISO 8601 format)
  GIT_COMMIT_TIMESTAMP=$(git log -1 --format=%aI 2>/dev/null || date -u +"%Y-%m-%dT%H:%M:%SZ")
  TIMESTAMP="$GIT_COMMIT_TIMESTAMP"
  # Convert to directory format: 2025-11-19T14:56:59+08:00 -> 2025-11-19_145659
  TIMESTAMP_DIR=$(echo "$GIT_COMMIT_TIMESTAMP" | awk -F'[T:+Z-]' '{printf "%s-%s-%s_%s%s%s", $1, $2, $3, $4, $5, $6}')
fi

# Create run directory
RUN_DIR="$HISTORY_DIR/${TIMESTAMP_DIR}_${GIT_COMMIT}"
mkdir -p "$RUN_DIR"

echo -e "${GREEN}  Commit: ${GIT_COMMIT}${NC}"
echo -e "${GREEN}  Branch: ${GIT_BRANCH}${NC}"
echo -e "${GREEN}  Message: ${GIT_MESSAGE}${NC}"
if [ "$GIT_DIRTY" = true ]; then
  echo -e "${YELLOW}  Status: DIRTY (uncommitted changes)${NC}"
else
  echo -e "${GREEN}  Status: Clean${NC}"
fi
echo -e "${GREEN}  Run ID: ${TIMESTAMP_DIR}_${GIT_COMMIT}${NC}"
echo ""

# Generate label from commit message
LABEL=$(echo "$GIT_MESSAGE" | sed 's/^[^:]*: //' | head -1)

# Get platform info
PLATFORM=$(uname -s)
COMPILER=$(gcc --version 2>/dev/null | head -1 || echo "gcc (version unknown)")

# Build flags
BUILD_FLAGS="-O3 -flto -march=native -DNDEBUG"

# Create metadata.json
cat > "$RUN_DIR/metadata.json" << EOF
{
  "timestamp": "$TIMESTAMP",
  "runId": "${TIMESTAMP_DIR}_${GIT_COMMIT}",
  "git": {
    "commit": "$GIT_COMMIT",
    "commitFull": "$GIT_COMMIT_FULL",
    "message": "$GIT_MESSAGE",
    "branch": "$GIT_BRANCH",
    "isDirty": $GIT_DIRTY
  },
  "build": {
    "compiler": "$COMPILER",
    "flags": "$BUILD_FLAGS",
    "platform": "$PLATFORM"
  },
  "label": "$LABEL"
}
EOF

echo -e "${CYAN}📝 Metadata saved to: ${RUN_DIR}/metadata.json${NC}"
echo ""

# Build benchmarks
echo -e "${CYAN}🔨 Building benchmarks...${NC}"
cd "$PROJECT_ROOT"
make clean > /dev/null 2>&1
make build-benchmarks 2>&1 | grep -E "(Building|✓|error|Error)" || true
echo ""

# Run performance benchmarks
if [ "$RUN_PERF" = true ]; then
  echo -e "${CYAN}⚡ Running performance benchmarks...${NC}"
  cd "$BENCHMARK_DIR"

  ./bin/bench_our_parser > /dev/null 2>&1 || {
    echo -e "${RED}Failed to run performance benchmark${NC}"
    exit 1
  }

  # Copy results
  cp "$RESULTS_DIR/json_parser_results.csv" "$RUN_DIR/performance.csv"
  echo -e "${GREEN}  ✓ Performance results saved${NC}"

  # Show quick summary
  echo -e "${YELLOW}  Quick stats:${NC}"
  tail -1 "$RUN_DIR/performance.csv" | awk -F',' '{printf "    large_array.json: %.2f ms (%.1f MB/s)\n", $6, $9}'
fi

# Run memory benchmarks
if [ "$RUN_MEMORY" = true ]; then
  echo -e "${CYAN}💾 Running memory benchmarks...${NC}"
  cd "$BENCHMARK_DIR"

  if [ ! -f "./bin/bench_memory" ]; then
    echo -e "${YELLOW}  ⚠️  Memory benchmark binary not found, skipping...${NC}"
  else
    ./bin/bench_memory > /dev/null 2>&1 || {
      echo -e "${YELLOW}  ⚠️  Memory benchmark failed, skipping...${NC}"
    }

    if [ -f "$RESULTS_DIR/memory_json_parser.csv" ]; then
      # Copy results
      cp "$RESULTS_DIR/memory_json_parser.csv" "$RUN_DIR/memory.csv"
      echo -e "${GREEN}  ✓ Memory results saved${NC}"

      # Show quick summary
      echo -e "${YELLOW}  Quick stats:${NC}"
      tail -1 "$RUN_DIR/memory.csv" | awk -F',' '{printf "    large_array.json: %d KB allocated, %d allocations\n", $5, $9}'
    fi
  fi
fi

echo ""

# Update index
echo -e "${CYAN}📇 Updating results index...${NC}"
"$SCRIPTS_DIR/generate_index.sh"
echo ""

# Update latest symlink
echo -e "${CYAN}🔗 Updating latest symlink...${NC}"
cd "$RESULTS_DIR"
rm -f latest
ln -s "history/${TIMESTAMP_DIR}_${GIT_COMMIT}" latest
echo -e "${GREEN}  ✓ results/latest -> history/${TIMESTAMP_DIR}_${GIT_COMMIT}${NC}"
echo ""

echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✅ Benchmark complete!${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "${CYAN}Results saved to:${NC}"
echo -e "  ${RUN_DIR}"
echo ""
echo -e "${CYAN}View visualization:${NC}"
echo -e "  ${YELLOW}make benchmark-view${NC}"
echo -e "  or open: ${YELLOW}benchmarks/visualize.html${NC}"
echo ""
