#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const ITERATIONS = 100;

function getTimeUs() {
    const [sec, nano] = process.hrtime();
    return sec * 1e6 + nano / 1e3;
}

function benchmarkFile(filepath) {
    const filename = path.basename(filepath);
    const content = fs.readFileSync(filepath, 'utf8');
    const fileSize = Buffer.byteLength(content);

    // Warmup
    JSON.parse(content);

    // Benchmark runs
    let totalTime = 0;
    let memBefore, memAfter;

    // Force GC if available
    if (global.gc) {
        global.gc();
    }

    memBefore = process.memoryUsage();

    for (let i = 0; i < ITERATIONS; i++) {
        const start = getTimeUs();
        JSON.parse(content);
        const end = getTimeUs();
        totalTime += (end - start);
    }

    memAfter = process.memoryUsage();

    const parseTimeMs = (totalTime / ITERATIONS) / 1000;
    const throughputMbps = (fileSize / (1024 * 1024)) / (parseTimeMs / 1000);

    const heapDelta = memAfter.heapUsed - memBefore.heapUsed;
    const externalDelta = memAfter.external - memBefore.external;

    return {
        filename,
        fileSize,
        parseTimeMs,
        throughputMbps,
        heapUsedDelta: heapDelta,
        externalDelta: externalDelta,
        heapTotal: memAfter.heapTotal,
        rss: memAfter.rss
    };
}

function main() {
    const args = process.argv.slice(2);

    if (args.length < 1) {
        console.error('Usage: node bench_nodejs.js <data_directory> [output_perf.csv] [output_mem.csv]');
        process.exit(1);
    }

    const dataDir = args[0];
    const outputPerf = args[1] || 'nodejs_performance.csv';
    const outputMem = args[2] || 'nodejs_memory.csv';

    console.log('Node.js JSON.parse() Benchmark');
    console.log('==============================\n');
    console.log(`Node.js version: ${process.version}\n`);

    // Create output CSV files
    const perfStream = fs.createWriteStream(outputPerf);
    const memStream = fs.createWriteStream(outputMem);

    perfStream.write('file,size_bytes,parse_time_ms,throughput_mbps\n');
    memStream.write('file,heap_used_delta,external_delta,heap_total,rss\n');

    // Read directory and benchmark each JSON file
    const files = fs.readdirSync(dataDir)
        .filter(file => file.endsWith('.json'))
        .sort();

    files.forEach(file => {
        const filepath = path.join(dataDir, file);
        console.log(`Benchmarking: ${file}`);

        try {
            const result = benchmarkFile(filepath);

            console.log(`  Parse time: ${result.parseTimeMs.toFixed(3)} ms`);
            console.log(`  Throughput: ${result.throughputMbps.toFixed(2)} MB/s`);
            console.log(`  Memory: heap delta ${result.heapUsedDelta} bytes, RSS ${result.rss} bytes\n`);

            perfStream.write(`${file},${result.fileSize},${result.parseTimeMs.toFixed(3)},${result.throughputMbps.toFixed(2)}\n`);
            memStream.write(`${file},${result.heapUsedDelta},${result.externalDelta},${result.heapTotal},${result.rss}\n`);
        } catch (error) {
            console.error(`  Error: ${error.message}\n`);
        }
    });

    perfStream.end();
    memStream.end();

    console.log(`Benchmark complete! Processed ${files.length} files.`);
    console.log('Results written to:');
    console.log(`  - ${outputPerf}`);
    console.log(`  - ${outputMem}`);
}

main();
