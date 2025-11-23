#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

function updateIndex(resultsDir) {
    const historyDir = path.join(resultsDir, 'history');
    const indexPath = path.join(resultsDir, 'index.json');

    if (!fs.existsSync(historyDir)) {
        console.error('History directory does not exist');
        process.exit(1);
    }

    const entries = [];
    const dirs = fs.readdirSync(historyDir, { withFileTypes: true })
        .filter(dirent => dirent.isDirectory())
        .map(dirent => dirent.name);

    for (const dir of dirs) {
        const metadataPath = path.join(historyDir, dir, 'metadata.json');
        if (fs.existsSync(metadataPath)) {
            const metadata = JSON.parse(fs.readFileSync(metadataPath, 'utf8'));
            entries.push({
                name: dir,
                commit: metadata.commit,
                timestamp: metadata.timestamp,
                git_status: metadata.git_status,
                node_version: metadata.node_version,
                platform: metadata.platform
            });
        }
    }

    // Sort by timestamp descending (newest first)
    entries.sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp));

    const index = {
        generated: new Date().toISOString(),
        count: entries.length,
        entries: entries
    };

    fs.writeFileSync(indexPath, JSON.stringify(index, null, 2));
    console.log(`Index updated with ${entries.length} entries`);
}

const resultsDir = process.argv[2] || path.join(__dirname, '../results');
updateIndex(resultsDir);
