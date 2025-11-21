#!/bin/bash

# Benchmark all optimization stages using commit timestamps
# Approach: Checkout old commits and bring current benchmark system with us

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Optimization stages (after API change in 460f39f)
declare -a STAGES=(
  "460f39f:Stage 1 - Baseline (Value-based Array Storage)"
  "91f097f:Stage 2 - Zero-Copy String Slices"
  "5886c3d:Stage 3 - Function Inlining Optimization"
  "HEAD:Stage 4 - Current (All Optimizations)"
)

TEMP_DIR="$SCRIPT_DIR/.benchmark_temp"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Benchmarking All Optimization Stages${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "${YELLOW}This will checkout each optimization commit and run benchmarks${NC}"
echo -e "${YELLOW}Results will use commit timestamps for chronological ordering${NC}"
echo ""

# Save current git state
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [ "$CURRENT_BRANCH" = "HEAD" ]; then
  CURRENT_COMMIT=$(git rev-parse HEAD)
fi

# Check for uncommitted changes
if ! git diff-index --quiet HEAD --; then
  echo -e "${RED}Error: You have uncommitted changes${NC}"
  echo -e "${YELLOW}Please commit or stash your changes before running this script${NC}"
  exit 1
fi

# Save the current benchmark system
echo -e "${CYAN}💾 Saving current benchmark system...${NC}"
mkdir -p "$TEMP_DIR"
# Use tar to preserve symlinks and handle edge cases
tar -czf "$TEMP_DIR/benchmarks_backup.tar.gz" benchmarks 2>/dev/null || {
  echo -e "${RED}Failed to backup benchmark system${NC}"
  exit 1
}
echo -e "${GREEN}  ✓ Benchmark system saved${NC}"
echo ""

# Function to benchmark a specific commit
benchmark_stage() {
  local commit=$1
  local stage_name=$2

  echo ""
  echo -e "${BLUE}========================================${NC}"
  echo -e "${BLUE}$stage_name${NC}"
  echo -e "${BLUE}Commit: $commit${NC}"
  echo -e "${BLUE}========================================${NC}"
  echo ""

  # Checkout the commit
  echo -e "${CYAN}📦 Checking out commit $commit...${NC}"
  git checkout "$commit" --quiet 2>/dev/null || {
    echo -e "${RED}  Failed to checkout commit${NC}"
    return 1
  }
  echo -e "${GREEN}  ✓ Checked out${NC}"

  # Restore the modern benchmark system
  echo -e "${CYAN}🔄 Restoring modern benchmark system...${NC}"
  rm -rf benchmarks
  tar -xzf "$TEMP_DIR/benchmarks_backup.tar.gz" -C . 2>/dev/null || {
    echo -e "${RED}  Failed to restore benchmark system${NC}"
    return 1
  }
  echo -e "${GREEN}  ✓ Benchmark system restored${NC}"

  # Run the benchmark with forced commit timestamp
  echo -e "${CYAN}⚡ Running benchmark...${NC}"
  cd benchmarks
  FORCE_COMMIT_TIMESTAMP=1 ./scripts/run_benchmark.sh
  cd ..

  echo -e "${GREEN}  ✓ Benchmark completed for $stage_name${NC}"
}

# Run benchmarks for each stage
for stage in "${STAGES[@]}"; do
  IFS=':' read -r commit name <<< "$stage"
  benchmark_stage "$commit" "$name"
done

# Return to original state
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Restoring Original State${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

if [ "$CURRENT_BRANCH" = "HEAD" ]; then
  echo -e "${CYAN}Returning to commit $CURRENT_COMMIT...${NC}"
  git checkout "$CURRENT_COMMIT" --quiet
else
  echo -e "${CYAN}Returning to branch $CURRENT_BRANCH...${NC}"
  git checkout "$CURRENT_BRANCH" --quiet
fi

# Cleanup
echo -e "${CYAN}🧹 Cleaning up...${NC}"
rm -rf "$TEMP_DIR"
echo -e "${GREEN}  ✓ Temporary files removed${NC}"

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}✅ All Benchmarks Complete!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "${CYAN}View results in your visualization:${NC}"
echo -e "  ${YELLOW}make benchmark-view${NC}"
echo -e "  or open: ${YELLOW}benchmarks/visualize.html${NC}"
echo ""
echo -e "${CYAN}Results are in:${NC}"
echo -e "  ${YELLOW}benchmarks/results/history/${NC}"
echo ""
echo -e "${CYAN}Note:${NC} Benchmark directories use commit timestamps,"
echo -e "so they appear in chronological order!"
echo ""
