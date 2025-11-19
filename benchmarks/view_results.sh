#!/bin/bash

# Open the benchmark visualization dashboard in default browser

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
HTML_FILE="$SCRIPT_DIR/visualize.html"

echo "Opening benchmark dashboard in your browser..."

# Detect OS and open accordingly
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    open "$HTML_FILE"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    xdg-open "$HTML_FILE" 2>/dev/null || sensible-browser "$HTML_FILE" 2>/dev/null || echo "Please open $HTML_FILE in your browser"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    # Windows
    start "$HTML_FILE"
else
    echo "Please open this file in your browser:"
    echo "$HTML_FILE"
fi

echo "✓ Dashboard opened!"
echo ""
echo "The dashboard shows:"
echo "  - Parse time comparison (before vs after)"
echo "  - Speedup factors for each test file"
echo "  - Throughput improvements"
echo "  - Memory allocation reduction"
echo "  - Performance vs Node.js V8"
