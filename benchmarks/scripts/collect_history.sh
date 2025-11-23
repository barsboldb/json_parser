#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BENCHMARK_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$BENCHMARK_DIR")"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Historical Benchmark Collection${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Save current state
cd "$PROJECT_DIR"
ORIGINAL_BRANCH=$(git branch --show-current)
ORIGINAL_COMMIT=$(git rev-parse HEAD)
HAS_CHANGES=false

if ! git diff-index --quiet HEAD --; then
    echo -e "${YELLOW}Warning: You have uncommitted changes${NC}"
    echo -e "${YELLOW}These will be stashed during the process${NC}"
    echo ""
    read -p "Continue? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Aborted."
        exit 1
    fi
    HAS_CHANGES=true
    git stash push -m "Temporary stash for historical benchmarks"
fi

# Get list of commits with messages
echo -e "${YELLOW}Fetching commit history...${NC}"
COMMITS=$(git log --oneline --no-merges | awk '{print $1}')
COMMIT_COUNT=$(echo "$COMMITS" | wc -l | tr -d ' ')

echo -e "${GREEN}Found $COMMIT_COUNT commits${NC}"
echo ""

# Parse options
START_FROM=""
LIMIT=""
SKIP_EXISTING=true

while [[ $# -gt 0 ]]; do
    case $1 in
        --start-from)
            START_FROM="$2"
            shift 2
            ;;
        --limit)
            LIMIT="$2"
            shift 2
            ;;
        --no-skip)
            SKIP_EXISTING=false
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--start-from <commit>] [--limit <n>] [--no-skip]"
            exit 1
            ;;
    esac
done

PROCESSED=0
SKIPPED=0
FAILED=0

# Function to restore original state
cleanup() {
    echo ""
    echo -e "${YELLOW}Restoring original state...${NC}"
    cd "$PROJECT_DIR"
    git checkout "$ORIGINAL_COMMIT" > /dev/null 2>&1
    if [ "$HAS_CHANGES" = true ]; then
        git stash pop > /dev/null 2>&1
    fi
    echo -e "${GREEN}✓ Restored to original state${NC}"
}

trap cleanup EXIT

# Process each commit
FOUND_START=false
if [ -z "$START_FROM" ]; then
    FOUND_START=true
fi

for COMMIT in $COMMITS; do
    # Check if we should start processing
    if [ "$FOUND_START" = false ]; then
        if [ "$COMMIT" = "$START_FROM" ]; then
            FOUND_START=true
        else
            continue
        fi
    fi

    # Check limit
    if [ -n "$LIMIT" ] && [ "$PROCESSED" -ge "$LIMIT" ]; then
        echo -e "${YELLOW}Reached limit of $LIMIT commits${NC}"
        break
    fi

    COMMIT_SHORT="${COMMIT:0:7}"
    RESULT_DIR="$BENCHMARK_DIR/results/history/$COMMIT_SHORT"

    # Skip if already exists
    if [ "$SKIP_EXISTING" = true ] && [ -d "$RESULT_DIR" ]; then
        echo -e "${BLUE}[$((PROCESSED + SKIPPED + FAILED + 1))/$COMMIT_COUNT]${NC} Skipping $COMMIT_SHORT (already exists)"
        SKIPPED=$((SKIPPED + 1))
        continue
    fi

    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}[$((PROCESSED + SKIPPED + FAILED + 1))/$COMMIT_COUNT] Processing commit: $COMMIT_SHORT${NC}"

    # Get commit message
    COMMIT_MSG=$(git log -1 --pretty=%B "$COMMIT" | head -n 1)
    echo -e "${YELLOW}$COMMIT_MSG${NC}"
    echo -e "${BLUE}========================================${NC}"

    # Checkout commit
    if ! git checkout "$COMMIT" > /dev/null 2>&1; then
        echo -e "${RED}✗ Failed to checkout commit${NC}"
        FAILED=$((FAILED + 1))
        continue
    fi

    # Try to build and benchmark
    cd "$PROJECT_DIR"

    # Clean previous build
    make clean > /dev/null 2>&1 || true

    # Try to build
    if ! make release > /dev/null 2>&1; then
        echo -e "${RED}✗ Build failed for commit $COMMIT_SHORT${NC}"
        FAILED=$((FAILED + 1))
        continue
    fi

    # Run benchmark
    if ! "$SCRIPT_DIR/run_benchmark.sh" --commit "$COMMIT" > /dev/null 2>&1; then
        echo -e "${RED}✗ Benchmark failed for commit $COMMIT_SHORT${NC}"
        FAILED=$((FAILED + 1))
        continue
    fi

    echo -e "${GREEN}✓ Benchmark complete for $COMMIT_SHORT${NC}"
    PROCESSED=$((PROCESSED + 1))
done

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}Historical collection complete!${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "Processed: ${GREEN}$PROCESSED${NC}"
echo -e "Skipped:   ${YELLOW}$SKIPPED${NC}"
echo -e "Failed:    ${RED}$FAILED${NC}"
echo ""
