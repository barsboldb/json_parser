#!/usr/bin/env node
/**
 * Generate large JSON test files for benchmarking
 */

const fs = require('fs');
const path = require('path');

const OUTPUT_DIR = path.join(__dirname, 'data');

// Ensure output directory exists
if (!fs.existsSync(OUTPUT_DIR)) {
  fs.mkdirSync(OUTPUT_DIR, { recursive: true });
}

// Generate large array (~50KB)
function generateLargeArray(size = 1000) {
  const arr = [];
  for (let i = 0; i < size; i++) {
    arr.push({
      id: i,
      name: `User ${i}`,
      email: `user${i}@example.com`,
      active: i % 2 === 0,
      score: Math.random() * 100,
      tags: ['tag1', 'tag2', 'tag3']
    });
  }
  return arr;
}

// Generate large object (~50KB)
function generateLargeObject(size = 500) {
  const obj = {
    metadata: {
      version: '1.0',
      timestamp: Date.now(),
      count: size
    },
    data: {}
  };

  for (let i = 0; i < size; i++) {
    obj.data[`key_${i}`] = {
      value: i,
      label: `Label ${i}`,
      nested: {
        a: i * 2,
        b: i * 3,
        c: `String ${i}`
      }
    };
  }

  return obj;
}

// Generate deeply nested structure (~20KB)
function generateDeeplyNested(depth = 50) {
  let obj = { value: 'deep', data: [1, 2, 3] };
  let current = obj;

  for (let i = 0; i < depth; i++) {
    current.nested = {
      level: i,
      name: `Level ${i}`,
      items: [i, i + 1, i + 2],
      metadata: { id: i }
    };
    current = current.nested;
  }

  return obj;
}

// Generate extra large array (~500KB)
function generateXLArray(size = 10000) {
  const arr = [];
  for (let i = 0; i < size; i++) {
    arr.push({
      id: i,
      uuid: `${Math.random().toString(36).substr(2, 9)}-${Date.now()}`,
      name: `Item ${i}`,
      description: `This is a longer description for item ${i} with more text to increase file size`,
      created_at: new Date(Date.now() - Math.random() * 10000000000).toISOString(),
      active: i % 3 !== 0,
      priority: Math.floor(Math.random() * 5) + 1,
      metadata: {
        views: Math.floor(Math.random() * 1000),
        likes: Math.floor(Math.random() * 100),
        tags: [`tag${i % 10}`, `category${i % 5}`, `type${i % 3}`]
      },
      scores: [
        Math.random() * 100,
        Math.random() * 100,
        Math.random() * 100
      ]
    });
  }
  return arr;
}

// Generate real-world API response (~100KB)
function generateRealWorldAPI() {
  return {
    status: 'success',
    timestamp: new Date().toISOString(),
    api_version: '2.1.0',
    request_id: Math.random().toString(36).substr(2, 16),
    data: {
      users: generateLargeArray(200),
      posts: Array(100).fill(0).map((_, i) => ({
        id: i,
        title: `Post ${i}`,
        content: `This is the content of post ${i}. `.repeat(5),
        author_id: Math.floor(Math.random() * 200),
        created_at: new Date(Date.now() - Math.random() * 10000000000).toISOString(),
        likes: Math.floor(Math.random() * 500),
        comments: Array(Math.floor(Math.random() * 20)).fill(0).map((_, j) => ({
          id: j,
          user_id: Math.floor(Math.random() * 200),
          text: `Comment ${j} on post ${i}`,
          timestamp: new Date(Date.now() - Math.random() * 1000000000).toISOString()
        }))
      })),
      statistics: {
        total_users: 200,
        total_posts: 100,
        active_today: Math.floor(Math.random() * 200),
        engagement_rate: Math.random() * 100
      }
    },
    meta: {
      page: 1,
      per_page: 100,
      total_pages: 10,
      has_more: true
    }
  };
}

// Write files
console.log('Generating benchmark test data...\n');

const files = [
  { name: 'large_array.json', data: generateLargeArray(1000) },
  { name: 'large_object.json', data: generateLargeObject(500) },
  { name: 'deeply_nested.json', data: generateDeeplyNested(50) },
  { name: 'xl_array.json', data: generateXLArray(10000) },
  { name: 'real_world_api.json', data: generateRealWorldAPI() }
];

files.forEach(({ name, data }) => {
  const json = JSON.stringify(data, null, 2);
  const filePath = path.join(OUTPUT_DIR, name);
  fs.writeFileSync(filePath, json);
  const sizeKB = (json.length / 1024).toFixed(2);
  console.log(`✓ Generated ${name} (${sizeKB} KB)`);
});

console.log('\n✓ All test data files generated successfully!');
console.log(`\nFiles saved to: ${OUTPUT_DIR}`);
