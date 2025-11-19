#!/bin/bash
# Generate index.json from all benchmark runs in history/
# This file is used by visualize.html for fast loading

set -e

BENCHMARK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HISTORY_DIR="$BENCHMARK_DIR/results/history"
INDEX_FILE="$BENCHMARK_DIR/results/index.json"

# Check if history directory exists
if [ ! -d "$HISTORY_DIR" ]; then
  echo "No history directory found. Creating empty index."
  echo '{"runs": [], "generated": "'$(date -u +"%Y-%m-%dT%H:%M:%SZ")'"}' > "$INDEX_FILE"
  exit 0
fi

# Start JSON array
echo '{'
echo '  "generated": "'$(date -u +"%Y-%m-%dT%H:%M:%SZ")'",'
echo '  "runs": ['

# Find all metadata.json files and extract info
FIRST=true
for METADATA in $(find "$HISTORY_DIR" -name "metadata.json" -type f | sort -r); do
  RUN_DIR=$(dirname "$METADATA")
  RUN_ID=$(basename "$RUN_DIR")

  # Check if required files exist
  PERF_FILE="$RUN_DIR/performance.csv"
  MEM_FILE="$RUN_DIR/memory.csv"

  HAS_PERF=false
  HAS_MEMORY=false

  if [ -f "$PERF_FILE" ]; then
    HAS_PERF=true
  fi

  if [ -f "$MEM_FILE" ]; then
    HAS_MEMORY=true
  fi

  # Add comma before all entries except the first
  if [ "$FIRST" = false ]; then
    echo ','
  fi
  FIRST=false

  # Extract key fields from metadata
  TIMESTAMP=$(grep '"timestamp"' "$METADATA" | sed 's/.*": "\(.*\)".*/\1/')
  COMMIT=$(grep '"commit"' "$METADATA" | head -1 | sed 's/.*": "\(.*\)".*/\1/')
  MESSAGE=$(grep '"message"' "$METADATA" | sed 's/.*": "\(.*\)".*/\1/')
  LABEL=$(grep '"label"' "$METADATA" | sed 's/.*": "\(.*\)".*/\1/')
  BRANCH=$(grep '"branch"' "$METADATA" | sed 's/.*": "\(.*\)".*/\1/')

  # Output run entry
  echo -n '    {'
  echo -n '"runId": "'$RUN_ID'",'
  echo -n '"timestamp": "'$TIMESTAMP'",'
  echo -n '"commit": "'$COMMIT'",'
  echo -n '"branch": "'$BRANCH'",'
  echo -n '"label": "'$LABEL'",'
  echo -n '"message": "'$MESSAGE'",'
  echo -n '"hasPerformance": '$HAS_PERF','
  echo -n '"hasMemory": '$HAS_MEMORY','
  echo -n '"path": "history/'$RUN_ID'"'
  echo -n '}'
done

# Close JSON
echo ''
echo '  ]'
echo '}'

# Write to index file
{
  echo '{'
  echo '  "generated": "'$(date -u +"%Y-%m-%dT%H:%M:%SZ")'",'
  echo '  "runs": ['

  FIRST=true
  for METADATA in $(find "$HISTORY_DIR" -name "metadata.json" -type f | sort -r); do
    RUN_DIR=$(dirname "$METADATA")
    RUN_ID=$(basename "$RUN_DIR")

    PERF_FILE="$RUN_DIR/performance.csv"
    MEM_FILE="$RUN_DIR/memory.csv"

    HAS_PERF=false
    HAS_MEMORY=false

    if [ -f "$PERF_FILE" ]; then
      HAS_PERF=true
    fi

    if [ -f "$MEM_FILE" ]; then
      HAS_MEMORY=true
    fi

    if [ "$FIRST" = false ]; then
      echo ','
    fi
    FIRST=false

    TIMESTAMP=$(grep '"timestamp"' "$METADATA" | sed 's/.*": "\(.*\)".*/\1/')
    COMMIT=$(grep '"commit"' "$METADATA" | head -1 | sed 's/.*": "\(.*\)".*/\1/')
    MESSAGE=$(grep '"message"' "$METADATA" | sed 's/.*": "\(.*\)".*/\1/' | sed 's/"/\\"/g')
    LABEL=$(grep '"label"' "$METADATA" | sed 's/.*": "\(.*\)".*/\1/' | sed 's/"/\\"/g')
    BRANCH=$(grep '"branch"' "$METADATA" | sed 's/.*": "\(.*\)".*/\1/')

    echo -n '    {'
    echo -n '"runId": "'$RUN_ID'",'
    echo -n '"timestamp": "'$TIMESTAMP'",'
    echo -n '"commit": "'$COMMIT'",'
    echo -n '"branch": "'$BRANCH'",'
    echo -n '"label": "'$LABEL'",'
    echo -n '"message": "'$MESSAGE'",'
    echo -n '"hasPerformance": '$HAS_PERF','
    echo -n '"hasMemory": '$HAS_MEMORY','
    echo -n '"path": "history/'$RUN_ID'"'
    echo -n '}'
  done

  echo ''
  echo '  ]'
  echo '}'
} > "$INDEX_FILE"

# Count runs
RUN_COUNT=$(find "$HISTORY_DIR" -name "metadata.json" -type f | wc -l | tr -d ' ')

echo "✓ Index generated: $INDEX_FILE"
echo "  Total runs indexed: $RUN_COUNT"
