#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const DATA_DIR = path.join(__dirname, '../data');

// Ensure data directory exists
if (!fs.existsSync(DATA_DIR)) {
    fs.mkdirSync(DATA_DIR, { recursive: true });
}

// Helper to generate random string
function randomString(length) {
    const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    let result = '';
    for (let i = 0; i < length; i++) {
        result += chars.charAt(Math.floor(Math.random() * chars.length));
    }
    return result;
}

// Helper to generate random number
function randomNumber() {
    return Math.random() * 1000000 - 500000;
}

// 1. Small JSON (1KB) - Simple object
function generateSmall() {
    return {
        name: "John Doe",
        age: 30,
        email: "john.doe@example.com",
        active: true,
        balance: 1234.56,
        tags: ["user", "premium", "verified"],
        address: {
            street: "123 Main St",
            city: "New York",
            zip: "10001"
        }
    };
}

// 2. Medium JSON (100KB) - Array of objects
function generateMedium() {
    const users = [];
    for (let i = 0; i < 500; i++) {
        users.push({
            id: i,
            username: `user_${i}`,
            email: `user${i}@example.com`,
            age: 18 + Math.floor(Math.random() * 50),
            active: Math.random() > 0.5,
            score: Math.random() * 1000,
            tags: Array.from({ length: 5 }, () => randomString(10)),
            metadata: {
                created: new Date().toISOString(),
                lastLogin: new Date().toISOString(),
                preferences: {
                    theme: "dark",
                    notifications: true,
                    language: "en"
                }
            }
        });
    }
    return { users, total: users.length };
}

// 3. Large JSON (1MB) - Large array with nested objects
function generateLarge() {
    const items = [];
    for (let i = 0; i < 5000; i++) {
        items.push({
            id: i,
            title: randomString(50),
            description: randomString(200),
            price: randomNumber(),
            inStock: Math.random() > 0.3,
            categories: Array.from({ length: 3 }, () => randomString(15)),
            ratings: Array.from({ length: 10 }, () => ({
                userId: Math.floor(Math.random() * 1000),
                score: Math.floor(Math.random() * 5) + 1,
                comment: randomString(100)
            })),
            specs: {
                weight: randomNumber(),
                dimensions: {
                    length: randomNumber(),
                    width: randomNumber(),
                    height: randomNumber()
                },
                materials: Array.from({ length: 3 }, () => randomString(20))
            }
        });
    }
    return { items, count: items.length };
}

// 4. Extra Large JSON (10MB) - Very large dataset
function generateXLarge() {
    const records = [];
    for (let i = 0; i < 30000; i++) {
        records.push({
            id: i,
            name: randomString(30),
            data: randomString(100),
            values: Array.from({ length: 20 }, () => randomNumber()),
            nested: {
                level1: {
                    level2: {
                        data: randomString(50),
                        numbers: Array.from({ length: 10 }, () => randomNumber())
                    }
                }
            }
        });
    }
    return { records, total: records.length };
}

// 5. Deeply nested (20 levels)
function generateDeeplyNested() {
    let obj = { value: "deep" };
    for (let i = 0; i < 20; i++) {
        obj = { level: i, data: randomString(20), nested: obj };
    }
    return obj;
}

// 6. Large array (pure array)
function generateLargeArray() {
    return Array.from({ length: 10000 }, (_, i) => ({
        id: i,
        value: randomNumber(),
        text: randomString(50)
    }));
}

// 7. Large object (many keys)
function generateLargeObject() {
    const obj = {};
    for (let i = 0; i < 5000; i++) {
        obj[`key_${i}`] = {
            value: randomNumber(),
            text: randomString(30),
            active: Math.random() > 0.5
        };
    }
    return obj;
}

// 8. Unicode and special characters
function generateUnicode() {
    return {
        english: "Hello World",
        chinese: "你好世界",
        japanese: "こんにちは世界",
        arabic: "مرحبا بالعالم",
        emoji: "👋🌍🚀💻🎉",
        mixed: "Hello 世界 🌍 مرحبا",
        specialChars: "!@#$%^&*()_+-=[]{}|;':\",./<>?",
        escaped: "Line1\nLine2\tTabbed\r\nWindows\\Path",
        array: ["emoji🎉", "中文", "العربية", "日本語"]
    };
}

// 9. Numeric edge cases
function generateNumericEdgeCases() {
    return {
        zero: 0,
        negativeZero: -0,
        integer: 42,
        negative: -42,
        float: 3.14159,
        negativeFloat: -3.14159,
        exponential: 1.23e10,
        negativeExponential: -1.23e-10,
        large: 1.7976931348623157e308,
        small: 2.2250738585072014e-308,
        arrays: [0, -0, 1, -1, 3.14, -3.14, 1e10, 1e-10]
    };
}

// 10. Empty structures
function generateEmptyStructures() {
    return {
        emptyObject: {},
        emptyArray: [],
        nullValue: null,
        nested: {
            empty: {},
            emptyArr: [],
            null: null
        },
        arrayOfEmpty: [{}, {}, []],
        mixed: {
            a: {},
            b: [],
            c: null,
            d: {
                e: {},
                f: []
            }
        }
    };
}

// 11. Long strings
function generateLongStrings() {
    return {
        short: randomString(100),
        medium: randomString(10000),
        long: randomString(100000),
        veryLong: randomString(1000000),
        array: Array.from({ length: 100 }, () => randomString(1000))
    };
}

// 12. Real-world: GitHub-style API response
function generateGitHubStyle() {
    return {
        id: 123456,
        node_id: "MDEwOlJlcG9zaXRvcnkxMjM0NTY=",
        name: "awesome-project",
        full_name: "user/awesome-project",
        private: false,
        owner: {
            login: "user",
            id: 789,
            avatar_url: "https://avatars.githubusercontent.com/u/789",
            url: "https://api.github.com/users/user",
            type: "User"
        },
        html_url: "https://github.com/user/awesome-project",
        description: "An awesome project for doing awesome things",
        fork: false,
        created_at: "2023-01-15T10:30:00Z",
        updated_at: "2023-11-20T15:45:00Z",
        pushed_at: "2023-11-20T15:45:00Z",
        size: 1024,
        stargazers_count: 42,
        watchers_count: 42,
        language: "JavaScript",
        forks_count: 7,
        open_issues_count: 3,
        default_branch: "main",
        topics: ["javascript", "nodejs", "awesome"],
        visibility: "public"
    };
}

// 13. Real-world: Package.json style
function generatePackageJson() {
    return {
        name: "my-awesome-package",
        version: "1.2.3",
        description: "A comprehensive package for awesome functionality",
        main: "index.js",
        scripts: {
            test: "jest",
            build: "webpack",
            start: "node index.js",
            dev: "nodemon index.js"
        },
        keywords: ["awesome", "package", "utility"],
        author: "Developer Name <dev@example.com>",
        license: "MIT",
        dependencies: {
            express: "^4.18.2",
            lodash: "^4.17.21",
            axios: "^1.5.0"
        },
        devDependencies: {
            jest: "^29.7.0",
            webpack: "^5.89.0",
            nodemon: "^3.0.1"
        },
        engines: {
            node: ">=14.0.0",
            npm: ">=6.0.0"
        }
    };
}

// 14. Real-world: Config file
function generateConfig() {
    return {
        server: {
            host: "localhost",
            port: 8080,
            ssl: {
                enabled: true,
                cert: "/path/to/cert.pem",
                key: "/path/to/key.pem"
            }
        },
        database: {
            host: "db.example.com",
            port: 5432,
            name: "myapp",
            user: "dbuser",
            pool: {
                min: 2,
                max: 10
            }
        },
        logging: {
            level: "info",
            format: "json",
            outputs: ["console", "file"],
            file: {
                path: "/var/log/myapp.log",
                maxSize: "10MB",
                maxFiles: 5
            }
        },
        features: {
            authentication: true,
            caching: true,
            rateLimit: {
                enabled: true,
                maxRequests: 100,
                windowMs: 60000
            }
        }
    };
}

// 15. Real-world: GeoJSON
function generateGeoJSON() {
    return {
        type: "FeatureCollection",
        features: Array.from({ length: 100 }, (_, i) => ({
            type: "Feature",
            geometry: {
                type: "Point",
                coordinates: [
                    -180 + Math.random() * 360,
                    -90 + Math.random() * 180
                ]
            },
            properties: {
                name: `Location ${i}`,
                description: randomString(50),
                category: ["restaurant", "hotel", "museum", "park"][Math.floor(Math.random() * 4)],
                rating: Math.random() * 5
            }
        }))
    };
}

// Generate all files
const files = [
    { name: 'small.json', generator: generateSmall, desc: 'Small JSON (~1KB)' },
    { name: 'medium.json', generator: generateMedium, desc: 'Medium JSON (~100KB)' },
    { name: 'large.json', generator: generateLarge, desc: 'Large JSON (~1MB)' },
    { name: 'xlarge.json', generator: generateXLarge, desc: 'Extra Large JSON (~10MB)' },
    { name: 'deeply_nested.json', generator: generateDeeplyNested, desc: 'Deeply nested (20 levels)' },
    { name: 'large_array.json', generator: generateLargeArray, desc: 'Large array (10k items)' },
    { name: 'large_object.json', generator: generateLargeObject, desc: 'Large object (5k keys)' },
    { name: 'unicode.json', generator: generateUnicode, desc: 'Unicode and special chars' },
    { name: 'numeric_edges.json', generator: generateNumericEdgeCases, desc: 'Numeric edge cases' },
    { name: 'empty_structures.json', generator: generateEmptyStructures, desc: 'Empty structures' },
    { name: 'long_strings.json', generator: generateLongStrings, desc: 'Long strings (up to 1MB)' },
    { name: 'github_api.json', generator: generateGitHubStyle, desc: 'GitHub-style API response' },
    { name: 'package_json.json', generator: generatePackageJson, desc: 'Package.json style' },
    { name: 'config.json', generator: generateConfig, desc: 'Config file' },
    { name: 'geojson.json', generator: generateGeoJSON, desc: 'GeoJSON' }
];

console.log('Generating test data files...\n');

files.forEach(({ name, generator, desc }) => {
    console.log(`Generating ${name} - ${desc}`);
    const data = generator();
    const json = JSON.stringify(data, null, 2);
    const filePath = path.join(DATA_DIR, name);
    fs.writeFileSync(filePath, json);
    const size = (json.length / 1024).toFixed(2);
    console.log(`  ✓ Generated ${name} (${size} KB)\n`);
});

console.log('All test data files generated successfully!');
