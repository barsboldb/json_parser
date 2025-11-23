#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BENCHMARK_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$BENCHMARK_DIR")"
DATA_DIR="$BENCHMARK_DIR/data"
RESULTS_DIR="$BENCHMARK_DIR/results"
HISTORY_DIR="$RESULTS_DIR/history"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Parse command line arguments
COMMIT_HASH=""
FORCE_TIMESTAMP=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --commit)
            COMMIT_HASH="$2"
            shift 2
            ;;
        --force-timestamp)
            FORCE_TIMESTAMP=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--commit <hash>] [--force-timestamp]"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}==================================${NC}"
echo -e "${BLUE}  JSON Parser Benchmark Suite${NC}"
echo -e "${BLUE}==================================${NC}"
echo ""

# Determine result directory name
if [ -n "$COMMIT_HASH" ]; then
    RESULT_NAME="${COMMIT_HASH:0:7}"
    GIT_STATUS="clean"
    CURRENT_COMMIT="$COMMIT_HASH"
elif [ "$FORCE_TIMESTAMP" = true ]; then
    RESULT_NAME="$(date +%Y%m%d_%H%M%S)"
    GIT_STATUS="timestamp"
    CURRENT_COMMIT="$(git rev-parse HEAD 2>/dev/null || echo 'unknown')"
else
    # Check if working tree is clean
    cd "$PROJECT_DIR"
    if git diff-index --quiet HEAD -- 2>/dev/null; then
        CURRENT_COMMIT=$(git rev-parse HEAD)
        RESULT_NAME="${CURRENT_COMMIT:0:7}"
        GIT_STATUS="clean"
    else
        RESULT_NAME="$(date +%Y%m%d_%H%M%S)"
        GIT_STATUS="dirty"
        CURRENT_COMMIT=$(git rev-parse HEAD 2>/dev/null || echo 'unknown')
    fi
fi

RESULT_DIR="$HISTORY_DIR/$RESULT_NAME"

echo -e "${YELLOW}Result directory: $RESULT_NAME${NC}"
echo -e "${YELLOW}Git status: $GIT_STATUS${NC}"
echo -e "${YELLOW}Commit: ${CURRENT_COMMIT:0:7}${NC}"
echo ""

# Create result directory
mkdir -p "$RESULT_DIR"

# Generate metadata
cat > "$RESULT_DIR/metadata.json" <<EOF
{
    "timestamp": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
    "commit": "$CURRENT_COMMIT",
    "git_status": "$GIT_STATUS",
    "result_name": "$RESULT_NAME",
    "node_version": "$(node --version)",
    "platform": "$(uname -s)",
    "arch": "$(uname -m)"
}
EOF

echo -e "${GREEN}✓ Metadata generated${NC}"

# Ensure test data exists
if [ ! -d "$DATA_DIR" ] || [ -z "$(ls -A "$DATA_DIR"/*.json 2>/dev/null)" ]; then
    echo -e "${YELLOW}Generating test data...${NC}"
    cd "$BENCHMARK_DIR"
    node src/generate_test_data.js
    echo -e "${GREEN}✓ Test data generated${NC}"
fi

# Compile benchmark binaries
echo -e "${YELLOW}Compiling C benchmark...${NC}"
cd "$PROJECT_DIR"
make clean > /dev/null 2>&1 || true
make release > /dev/null 2>&1

# Compile mem_track.c WITHOUT BENCHMARK_MEMORY_TRACKING (to get real malloc pointers)
gcc -O3 -march=native -c \
    -I"$BENCHMARK_DIR/include" \
    -o "$BENCHMARK_DIR/bin/mem_track.o" \
    "$BENCHMARK_DIR/src/mem_track.c"

# Compile everything else WITH BENCHMARK_MEMORY_TRACKING (to redirect malloc to counters)
gcc -O3 -march=native -flto \
    -DBENCHMARK_MEMORY_TRACKING \
    -I"$PROJECT_DIR/include" \
    -I"$BENCHMARK_DIR/include" \
    -o "$BENCHMARK_DIR/bin/bench_parser" \
    "$BENCHMARK_DIR/src/bench_parser.c" \
    "$BENCHMARK_DIR/bin/mem_track.o" \
    "$PROJECT_DIR/src/lexer.c" \
    "$PROJECT_DIR/src/parser.c" \
    "$PROJECT_DIR/src/json.c"

echo -e "${GREEN}✓ C benchmark compiled${NC}"

# Run C parser benchmark
echo ""
echo -e "${BLUE}Running C parser benchmark...${NC}"
"$BENCHMARK_DIR/bin/bench_parser" \
    "$DATA_DIR" \
    "$RESULT_DIR/performance.csv" \
    "$RESULT_DIR/memory.csv"
echo -e "${GREEN}✓ C benchmark complete${NC}"

# Run Node.js benchmark
echo ""
echo -e "${BLUE}Running Node.js benchmark...${NC}"
node "$BENCHMARK_DIR/src/bench_nodejs.js" \
    "$DATA_DIR" \
    "$RESULT_DIR/nodejs_performance.csv" \
    "$RESULT_DIR/nodejs_memory.csv"
echo -e "${GREEN}✓ Node.js benchmark complete${NC}"

# Update latest symlink
cd "$RESULTS_DIR"
rm -f latest
ln -sf "history/$RESULT_NAME" latest

echo ""
echo -e "${GREEN}✓ Updated latest symlink${NC}"

# Update index.json
echo -e "${YELLOW}Updating index...${NC}"
node "$SCRIPT_DIR/update_index.js" "$RESULTS_DIR"
echo -e "${GREEN}✓ Index updated${NC}"

echo ""
echo -e "${BLUE}==================================${NC}"
echo -e "${GREEN}Benchmark complete!${NC}"
echo -e "${BLUE}==================================${NC}"
echo ""
echo -e "Results saved to: ${YELLOW}$RESULT_DIR${NC}"
echo ""
echo -e "View results:"
echo -e "  ${BLUE}make benchmark-view${NC}"
echo ""
