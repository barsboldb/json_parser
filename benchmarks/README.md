# JSON Parser Benchmarks

Comprehensive performance and memory benchmarking suite with git-tracked history and interactive visualization.

## Quick Start

```bash
# Run full benchmark suite (auto-captures git metadata)
make benchmark

# View interactive visualization
make benchmark-view
# Opens http://localhost:8000/visualize.html
# Press Ctrl+C to stop server
```

## Commands

```bash
make benchmark              # Run performance + memory benchmarks
make benchmark-perf         # Performance only
make benchmark-memory       # Memory only
make benchmark-view         # Start web server & open visualization
```

## Features

- ✅ **Auto-captures git metadata** - commit hash, message, branch, timestamp
- ✅ **Interactive visualization** - 4 chart types, toggleable runs, metric comparison
- ✅ **Historical tracking** - all runs preserved in `results/history/`
- ✅ **Export data** - download comparisons as CSV

## Visualization

The dashboard provides:
- **Timeline Chart** - Performance evolution over time
- **Commits Chart** - Scatter plot by commit date
- **Compare Chart** - Side-by-side bar comparison
- **Delta Chart** - % improvement/regression between runs

Toggle runs, switch metrics (parse time, throughput, memory), and compare across test files.

## Directory Structure

```
benchmarks/
├── README.md                           # This file
├── visualize.html                      # Interactive dashboard
├── scripts/
│   ├── run_benchmark.sh                # Main benchmark runner
│   ├── generate_index.sh               # Index generator
│   └── serve_visualization.sh          # Web server for visualization
├── results/
│   ├── history/                        # All benchmark runs
│   │   └── 2025-11-19_030217_0983838/  # Timestamped run directory
│   │       ├── metadata.json           # Git + build metadata
│   │       ├── performance.csv         # Performance data
│   │       └── memory.csv              # Memory data
│   ├── index.json                      # Index for visualization
│   └── latest -> history/...           # Symlink to latest run
└── archive/
    └── MEMORY_OPTIMIZATION_COMPARISON.md  # Historical optimization analysis
```

## What Gets Tracked

Each benchmark run captures:
- Git commit hash (full + short)
- Commit message (becomes run label)
- Branch name
- Timestamp
- Compiler info & build flags
- Dirty status (uncommitted changes)
- Performance metrics (parse time, throughput)
- Memory metrics (allocations, peak usage, RSS)

## Workflow Example

```bash
# 1. Make optimization
vim src/parser.c

# 2. Commit
git commit -m "perf: optimize with hash tables"

# 3. Benchmark (auto-captures commit info)
make benchmark

# 4. View results
make benchmark-view
```

## Troubleshooting

**Visualization shows error**: Run `make benchmark-view` (not just opening the HTML - it needs a web server for CORS)

**No data showing**: Run `make benchmark` first to create benchmark data

**Port 8000 in use**: Kill it with `lsof -ti:8000 | xargs kill -9`

## Legacy Files

Old benchmark scripts remain for compatibility:
- `run_benchmarks.sh` - Old performance runner
- `run_memory_benchmarks.sh` - Old memory runner

Use `make benchmark` instead for the new system with git tracking.
