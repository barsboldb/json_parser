#!/usr/bin/env node
/**
 * Benchmark JavaScript's native JSON.parse()
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

// Benchmark a file
function benchmarkFile(filepath, iterations) {
  const json = readFile(filepath);
  if (!json) return null;

  const result = {
    filename: filepath,
    iterations: iterations,
    totalTimeMs: 0,
    avgTimeMs: 0,
    minTimeMs: Infinity,
    maxTimeMs: 0,
    throughputMBs: 0,
    fileSizeBytes: json.length
  };

  // Warm-up run
  JSON.parse(json);

  // Benchmark iterations
  for (let i = 0; i < iterations; i++) {
    const start = performance.now();
    JSON.parse(json);
    const end = performance.now();

    const elapsed = end - start;
    result.totalTimeMs += elapsed;
    result.minTimeMs = Math.min(result.minTimeMs, elapsed);
    result.maxTimeMs = Math.max(result.maxTimeMs, elapsed);
  }

  result.avgTimeMs = result.totalTimeMs / iterations;

  // Calculate throughput (MB/s)
  const avgTimeS = result.avgTimeMs / 1000.0;
  const fileSizeMB = result.fileSizeBytes / (1024 * 1024);
  result.throughputMBs = fileSizeMB / avgTimeS;

  return result;
}

// Print result
function printResult(result) {
  console.log(`\nFile: ${result.filename}`);
  console.log(`  Size: ${(result.fileSizeBytes / 1024).toFixed(2)} KB`);
  console.log(`  Iterations: ${result.iterations}`);
  console.log(`  Total time: ${result.totalTimeMs.toFixed(2)} ms`);
  console.log(`  Avg time: ${result.avgTimeMs.toFixed(4)} ms`);
  console.log(`  Min time: ${result.minTimeMs.toFixed(4)} ms`);
  console.log(`  Max time: ${result.maxTimeMs.toFixed(4)} ms`);
  console.log(`  Throughput: ${result.throughputMBs.toFixed(2)} MB/s`);
}

// Save results to CSV
function saveCSV(results, outputFile) {
  const rows = [
    'Library,Filename,Size(KB),Iterations,TotalTime(ms),AvgTime(ms),MinTime(ms),MaxTime(ms),Throughput(MB/s)'
  ];

  results.forEach(r => {
    rows.push(
      `nodejs_json_parse,${r.filename},${(r.fileSizeBytes / 1024).toFixed(2)},${r.iterations},${r.totalTimeMs.toFixed(2)},${r.avgTimeMs.toFixed(4)},${r.minTimeMs.toFixed(4)},${r.maxTimeMs.toFixed(4)},${r.throughputMBs.toFixed(2)}`
    );
  });

  fs.writeFileSync(outputFile, rows.join('\n'));
  console.log(`\n✓ Results saved to: ${outputFile}`);
}

// Main
console.log('===========================================');
console.log('JSON Parser Benchmark - Node.js JSON.parse()');
console.log('===========================================');
console.log(`Node.js version: ${process.version}`);
console.log(`V8 version: ${process.versions.v8}`);

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

const iterationsMap = [
  10000,  // simple.json (< 1KB)
  10000,  // array.json (< 1KB)
  5000,   // nested.json (< 1KB)
  5000,   // complex.json (< 1KB)
  5000,   // edge_cases.json (< 1KB)
  500,    // large_array.json (~200KB)
  500,    // large_object.json (~80KB)
  1000,   // deeply_nested.json (~40KB)
  500     // real_world_api.json (~250KB)
];

const results = [];

testFiles.forEach((file, i) => {
  console.log(`\nBenchmarking: ${file}`);
  const result = benchmarkFile(file, iterationsMap[i]);
  if (result) {
    printResult(result);
    results.push(result);
  }
});

console.log('\n===========================================');
console.log('Benchmark Complete!');
console.log('===========================================');

// Ensure results directory exists
const resultsDir = path.join(benchmarksDir, 'results');
if (!fs.existsSync(resultsDir)) {
  fs.mkdirSync(resultsDir, { recursive: true });
}

saveCSV(results, path.join(resultsDir, 'nodejs_results.csv'));
