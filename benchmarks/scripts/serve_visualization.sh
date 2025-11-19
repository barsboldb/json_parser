#!/bin/bash
# Start web server and open browser for visualization

BENCHMARK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "🚀 Starting local web server for benchmark visualization..."
echo ""
echo "📊 Opening browser to: http://localhost:8000/visualize.html"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""

# Open browser after a short delay
(sleep 2 && open "http://localhost:8000/visualize.html" 2>/dev/null || xdg-open "http://localhost:8000/visualize.html" 2>/dev/null) &

# Start server
cd "$BENCHMARK_DIR"
python3 -m http.server 8000 2>/dev/null || python -m SimpleHTTPServer 8000
