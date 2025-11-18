#!/usr/bin/env node
/**
 * Benchmark Node.js JSON.parse() memory usage
 */

const fs = require('fs');
const path = require('path');
const { performance } = require('perf_hooks');

// Read file
function readFile(filepath) {
  try {
    return fs.readFileSync(filepath, 'utf8');
  } catch (err) {
    console.error(`Failed to read file: ${filepath}`);
    return null;
  }
}

// Force garbage collection if available
function forceGC() {
  if (global.gc) {
    global.gc();
  }
}

// Get memory usage in KB
function getMemoryUsage() {
  const mem = process.memoryUsage();
  return {
    rss: Math.round(mem.rss / 1024), // KB
    heapTotal: Math.round(mem.heapTotal / 1024), // KB
    heapUsed: Math.round(mem.heapUsed / 1024), // KB
    external: Math.round(mem.external / 1024) // KB
  };
}

// Benchmark memory for a file
function benchmarkMemory(filepath) {
  const json = readFile(filepath);
  if (!json) return null;

  const result = {
    filename: filepath,
    fileSizeBytes: json.length,
    fileSizeKB: json.length / 1024,
    parseTimeMs: 0,

    // Memory before parsing
    rssBeforeKB: 0,
    heapUsedBeforeKB: 0,
    heapTotalBeforeKB: 0,

    // Memory after parsing
    rssAfterKB: 0,
    heapUsedAfterKB: 0,
    heapTotalAfterKB: 0,

    // Deltas
    rssDeltaKB: 0,
    heapUsedDeltaKB: 0,
    heapTotalDeltaKB: 0,

    // Memory retained after GC
    rssAfterGCKB: 0,
    heapUsedAfterGCKB: 0,
    heapRetainedKB: 0,

    // Overhead
    overheadRatio: 0
  };

  // Force GC before starting
  forceGC();

  // Small delay to let GC settle
  const start = Date.now();
  while (Date.now() - start < 50) { /* wait */ }

  // Get memory before parsing
  const memBefore = getMemoryUsage();
  result.rssBeforeKB = memBefore.rss;
  result.heapUsedBeforeKB = memBefore.heapUsed;
  result.heapTotalBeforeKB = memBefore.heapTotal;

  // Parse with timing
  const parseStart = performance.now();
  const parsed = JSON.parse(json);
  const parseEnd = performance.now();
  result.parseTimeMs = parseEnd - parseStart;

  // Get memory after parsing
  const memAfter = getMemoryUsage();
  result.rssAfterKB = memAfter.rss;
  result.heapUsedAfterKB = memAfter.heapUsed;
  result.heapTotalAfterKB = memAfter.heapTotal;

  // Calculate deltas
  result.rssDeltaKB = result.rssAfterKB - result.rssBeforeKB;
  result.heapUsedDeltaKB = result.heapUsedAfterKB - result.heapUsedBeforeKB;
  result.heapTotalDeltaKB = result.heapTotalAfterKB - result.heapTotalBeforeKB;

  // Clear reference and force GC to see retained memory
  // Keep reference to prevent optimization
  const temp = parsed;
  forceGC();

  // Get memory after GC
  const memAfterGC = getMemoryUsage();
  result.rssAfterGCKB = memAfterGC.rss;
  result.heapUsedAfterGCKB = memAfterGC.heapUsed;
  result.heapRetainedKB = result.heapUsedAfterGCKB - result.heapUsedBeforeKB;

  // Calculate overhead ratio (heap used delta / file size)
  result.overheadRatio = result.heapUsedDeltaKB / result.fileSizeKB;

  return result;
}

// Print result
function printResult(result) {
  console.log(`\nFile: ${result.filename}`);
  console.log(`  File size: ${result.fileSizeKB.toFixed(2)} KB`);
  console.log(`  Parse time: ${result.parseTimeMs.toFixed(4)} ms`);
  console.log(`\n  Heap Memory:`);
  console.log(`    Before: ${result.heapUsedBeforeKB} KB`);
  console.log(`    After: ${result.heapUsedAfterKB} KB`);
  console.log(`    Delta: ${result.heapUsedDeltaKB} KB`);
  console.log(`    After GC: ${result.heapUsedAfterGCKB} KB`);
  console.log(`    Retained: ${result.heapRetainedKB} KB`);
  console.log(`    Overhead ratio: ${result.overheadRatio.toFixed(2)}x (heap delta / file size)`);
  console.log(`\n  Process Memory (RSS):`);
  console.log(`    Before: ${result.rssBeforeKB} KB`);
  console.log(`    After: ${result.rssAfterKB} KB`);
  console.log(`    Delta: ${result.rssDeltaKB} KB`);
  console.log(`    After GC: ${result.rssAfterGCKB} KB`);
}

// Save results to CSV
function saveCSV(results, outputFile) {
  const rows = [
    'Library,Filename,FileSize(KB),ParseTime(ms),HeapBefore(KB),HeapAfter(KB),HeapDelta(KB),HeapRetained(KB),OverheadRatio,RSS_Before(KB),RSS_After(KB),RSS_Delta(KB),RSS_AfterGC(KB)'
  ];

  results.forEach(r => {
    rows.push(
      `nodejs_json_parse,${r.filename},${r.fileSizeKB.toFixed(2)},${r.parseTimeMs.toFixed(4)},${r.heapUsedBeforeKB},${r.heapUsedAfterKB},${r.heapUsedDeltaKB},${r.heapRetainedKB},${r.overheadRatio.toFixed(2)},${r.rssBeforeKB},${r.rssAfterKB},${r.rssDeltaKB},${r.rssAfterGCKB}`
    );
  });

  fs.writeFileSync(outputFile, rows.join('\n'));
  console.log(`\n✓ Memory results saved to: ${outputFile}`);
}

// Main
console.log('===========================================');
console.log('JSON Parser Memory Benchmark - Node.js');
console.log('===========================================');
console.log(`Node.js version: ${process.version}`);
console.log(`V8 version: ${process.versions.v8}`);

if (!global.gc) {
  console.log('\n⚠️  WARNING: Run with --expose-gc flag for accurate GC measurements');
  console.log('   Example: node --expose-gc bench_memory_js.js\n');
} else {
  console.log('✓ GC exposed for accurate measurements\n');
}

// Get benchmarks directory (parent of src)
const benchmarksDir = path.join(__dirname, '..');

const testFiles = [
  path.join(benchmarksDir, '../samples/simple.json'),
  path.join(benchmarksDir, '../samples/array.json'),
  path.join(benchmarksDir, '../samples/nested.json'),
  path.join(benchmarksDir, '../samples/complex.json'),
  path.join(benchmarksDir, '../samples/edge_cases.json'),
  path.join(benchmarksDir, 'data/large_array.json'),
  path.join(benchmarksDir, 'data/large_object.json'),
  path.join(benchmarksDir, 'data/deeply_nested.json'),
  path.join(benchmarksDir, 'data/real_world_api.json')
];

const results = [];

testFiles.forEach((file) => {
  console.log(`\nBenchmarking: ${file}`);
  const result = benchmarkMemory(file);
  if (result) {
    printResult(result);
    results.push(result);
  }
});

console.log('\n===========================================');
console.log('Memory Benchmark Complete!');
console.log('===========================================');

// Ensure results directory exists
const resultsDir = path.join(benchmarksDir, 'results');
if (!fs.existsSync(resultsDir)) {
  fs.mkdirSync(resultsDir, { recursive: true });
}

saveCSV(results, path.join(resultsDir, 'memory_nodejs.csv'));
