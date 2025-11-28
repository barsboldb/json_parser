# JSON Parser in C

A lightweight JSON lexer and parser implementation written in C from scratch. This project provides a complete tokenization system for JSON data with proper error handling and memory management.

## Features

- **Complete JSON Lexical Analysis**: Tokenizes all JSON data types including objects, arrays, strings, numbers, booleans, and null values
- **Full Parser Implementation**: Builds complete JSON AST with support for all JSON types
- **Memory Pool Allocation**: Efficient memory management with custom pool allocator
- **Robust Number Parsing**: Handles integers, floats, scientific notation, and negative numbers according to JSON specification
- **String Escape Handling**: Properly processes escape sequences in JSON strings
- **Error Reporting**: Provides line and column information for debugging
- **Memory Safe**: Proper memory allocation and deallocation with pool-based management
- **Standards Compliant**: Follows JSON specification for parsing rules
- **Comprehensive Benchmarking**: Full benchmark suite comparing performance against Node.js V8
- **Visualization Dashboard**: Interactive HTML dashboard for benchmark results
- **Comprehensive Testing**: Built-in test framework for reliable development

## Project Structure

```
.
├── include/
│   ├── json.h          # JSON value structures and types
│   ├── lexer.h         # Lexer interface and token definitions
│   ├── parser.h        # Parser interface
│   └── mem_pool.h      # Memory pool allocator
├── src/
│   ├── main.c          # Example usage and testing
│   ├── lexer.c         # Lexer implementation
│   ├── json.c          # JSON value operations
│   ├── parser.c        # Parser implementation
│   └── mem_pool.c      # Memory pool implementation
├── tests/
│   ├── test_framework.h # Testing framework header
│   └── test_*.c        # Individual test files
├── benchmarks/
│   ├── src/            # Benchmark source code
│   ├── data/           # Test JSON files
│   ├── results/        # Benchmark results
│   ├── scripts/        # Benchmark runner scripts
│   └── visualize.html  # Interactive visualization dashboard
├── samples/
│   ├── simple.json     # Basic JSON example
│   ├── array.json      # Array examples
│   └── nested.json     # Complex nested structures
├── scripts/
│   ├── build           # Build script
│   ├── test            # Test runner script
│   └── sync            # Git synchronization script
├── Makefile            # Build automation
├── POOL_OPTIMIZATION.md      # Pool optimization guide
└── PERFORMANCE_ANALYSIS.md   # Performance analysis
```

## Supported JSON Types

The parser recognizes all standard JSON data types:

- **Objects**: `{"key": "value"}`
- **Arrays**: `[1, 2, 3]`
- **Strings**: `"hello world"`
- **Numbers**: `123`, `-45.67`, `1.23e-4`
- **Booleans**: `true`, `false`
- **Null**: `null`

## Token Types

The lexer generates the following token types:

| Token | Description |
|-------|-------------|
| `TOKEN_LBRACE` | Left brace `{` |
| `TOKEN_RBRACE` | Right brace `}` |
| `TOKEN_LBRACKET` | Left bracket `[` |
| `TOKEN_RBRACKET` | Right bracket `]` |
| `TOKEN_COLON` | Colon `:` |
| `TOKEN_COMMA` | Comma `,` |
| `TOKEN_STRING` | String literals |
| `TOKEN_NUMBER` | Numeric values |
| `TOKEN_TRUE` | Boolean true |
| `TOKEN_FALSE` | Boolean false |
| `TOKEN_NULL` | Null value |
| `TOKEN_EOF` | End of input |
| `TOKEN_ERROR` | Parse errors |

## Building and Running

### Prerequisites

- GCC compiler
- Zsh shell (for build scripts)
- Standard C libraries

### Quick Start

```bash
# Make scripts executable
chmod +x scripts/*

# Build the project
./scripts/build

# Build and execute main program
./scripts/build -e

# Run all tests
./scripts/test

# Run tests with cleanup
./scripts/test -c

# Run specific test
./scripts/test test_lexer

# List available tests
./scripts/test -l
```

### Using Makefile

```bash
# Build release version
make

# Build debug version
make debug

# Run parser on a file
make run

# Run benchmarks
make benchmark

# View benchmark results
make benchmark-view

# Clean build artifacts
make clean
```

### Manual Build

```bash
# Create dist directory
mkdir -p dist

# Compile main program
gcc ./src/*.c -Iinclude -o ./dist/main

# Compile and run a specific test
gcc ./tests/test_lexer.c ./src/*.c -Iinclude -Itests -o ./dist/test_lexer
./dist/test_lexer
```

## Usage

### Basic Parser Usage

```c
#include "include/parser.h"

int main() {
    const char* json_input = "{\"name\": \"John\", \"age\": 30}";

    lexer_t lexer = lexer_init(json_input);
    parser_t parser = parser_init(&lexer);

    parser.current_token = next_token(&lexer);
    json_value_t value = parse(&parser);

    if (!parser.has_error) {
        // Successfully parsed JSON
        json_value_print(&value);
    } else {
        printf("Error: %s\n", parser.error_message);
    }

    lexer_free(&lexer);
    parser_free(&parser);

    return 0;
}
```

### Basic Lexer Usage

```c
#include "include/lexer.h"

int main() {
    const char* json_input = "{\"name\": \"John\", \"age\": 30}";

    lexer_t lexer = lexer_init(json_input);

    token_t token;
    while ((token = next_token(&lexer)).type != TOKEN_EOF) {
        print_token(&token);
        token_free(&token);  // Important: free token memory
    }

    return 0;
}
```

### Sample Output

For the input `{"key": 123, "name": "barsbold", "is_user": false}`, the lexer produces:

```
TOKEN_LBRACE col: 1 lin: 1 lexeme: {
TOKEN_STRING col: 2 lin: 1 lexeme: key
TOKEN_COLON col: 7 lin: 1 lexeme: :
TOKEN_NUMBER col: 9 lin: 1 lexeme: 123
TOKEN_COMMA col: 12 lin: 1 lexeme: ,
TOKEN_STRING col: 16 lin: 1 lexeme: name
TOKEN_COLON col: 22 lin: 1 lexeme: :
TOKEN_STRING col: 24 lin: 1 lexeme: barsbold
TOKEN_COMMA col: 34 lin: 1 lexeme: ,
TOKEN_STRING col: 38 lin: 1 lexeme: is_user
TOKEN_COLON col: 47 lin: 1 lexeme: :
TOKEN_FALSE col: 49 lin: 1 lexeme: false
TOKEN_RBRACE col: 54 lin: 1 lexeme: }
```

## Testing

The project includes a comprehensive testing framework:

### Test Framework Features

- **Colored Output**: Visual feedback for test results
- **Individual Test Execution**: Run specific tests
- **Test Discovery**: Automatic detection of test files
- **Memory Cleanup**: Optional cleanup of test binaries
- **Detailed Reporting**: Summary of passed/failed tests

### Running Tests

```bash
# Run all tests
./scripts/test

# Run with cleanup
./scripts/test -c

# Run specific test
./scripts/test test_lexer

# List available tests
./scripts/test -l

# Get help
./scripts/test -h
```

### Writing Tests

Tests should be placed in the `tests/` directory with the naming convention `test_*.c`. Use the test framework header:

```c
#include "test_framework.h"
#include "../include/lexer.h"

void test_basic_tokenization() {
    // Test implementation
    assert_token_type(token, TOKEN_STRING);
}

int main() {
    test_basic_tokenization();
    printf("All tests passed!\n");
    return 0;
}
```

## API Reference

### Parser Functions

#### `parser_t parser_init(lexer_t *lexer)`
Initializes a parser with the given lexer. Creates a memory pool for allocations.

#### `json_value_t parse(parser_t *parser)`
Parses the input and returns a JSON value.

#### `void parser_free(parser_t *parser)`
Frees the parser and its associated memory pool.

### Lexer Functions

#### `lexer_t lexer_init(const char *input)`
Initializes a lexer with the given JSON input string.

#### `token_t next_token(lexer_t *lexer)`
Returns the next token from the input stream.

#### `void token_free(token_t *token)`
Frees memory allocated for a token's lexeme.

#### `void print_token(token_t *token)`
Prints token information for debugging.

### JSON Value Functions

#### `void json_value_print(json_value_t *value)`
Prints a JSON value in formatted JSON.

#### `void json_array_push(json_value_t *arr, json_value_t val)`
Adds an element to a JSON array.

#### `void json_object_insert(json_value_t *obj, const char *key, json_value_t val)`
Inserts a key-value pair into a JSON object.

### Memory Pool Functions

#### `mem_pool_t *pool_create(void)`
Creates a new memory pool.

#### `void *pool_alloc(mem_pool_t *pool, size_t size)`
Allocates memory from the pool.

#### `void pool_destroy(mem_pool_t *pool)`
Destroys the pool and frees all allocations.

#### `size_t pool_bytes_used(mem_pool_t *pool)`
Returns the number of bytes currently used in the pool.

#### `size_t pool_bytes_allocated(mem_pool_t *pool)`
Returns the total number of bytes allocated by the pool.

## Scripts Reference

### Build Script (`./scripts/build`)

```bash
./scripts/build        # Build only
./scripts/build -e     # Build and execute
```

### Test Script (`./scripts/test`)

```bash
./scripts/test                    # Run all tests
./scripts/test -c                 # Run all tests with cleanup
./scripts/test test_lexer         # Run specific test
./scripts/test -c test_lexer      # Run specific test with cleanup
./scripts/test -l                 # List available tests
./scripts/test -h                 # Show help
```

### Sync Script (`./scripts/sync`)

Synchronizes with the remote Git repository:

```bash
./scripts/sync
```

## Error Handling

The lexer provides detailed error information:

- **Line and Column Numbers**: Exact position of errors in the input
- **Error Tokens**: `TOKEN_ERROR` type for invalid input
- **Descriptive Messages**: Meaningful error descriptions (e.g., "Unterminated string")

## Memory Management

### Memory Pool

The parser uses a custom memory pool allocator for efficient memory management:

- **Pool Allocation**: All parsed JSON values are allocated from a memory pool
- **Automatic Cleanup**: Calling `parser_free()` deallocates all pool memory at once
- **Zero Leaks**: No need to manually free individual JSON values
- **Performance**: Faster than malloc/free for many small allocations
- **Cache Friendly**: Contiguous memory allocation improves cache performance

### Token Memory

- All tokens allocate memory for their lexeme strings
- **Important**: Always call `token_free()` for each token to prevent memory leaks
- The lexer creates a copy of the input string for safe processing

### Best Practices

```c
// Good: Parser automatically manages memory
lexer_t lexer = lexer_init(json_input);
parser_t parser = parser_init(&lexer);
json_value_t value = parse(&parser);

// Use the value...

parser_free(&parser);  // Frees all pool allocations
lexer_free(&lexer);

// Bad: Don't manually free pool-allocated values
// json_value_free(&value);  // ❌ Already freed by parser_free()
```

## Development Status

- ✅ **Lexer**: Complete with full JSON tokenization
- ✅ **Parser**: Full implementation with memory pool support
- ✅ **JSON Values**: Complete data structures and operations
- ✅ **Memory Pool**: Custom allocator for efficient memory management
- ✅ **Build System**: Makefile + automated build scripts
- ✅ **Testing Framework**: Comprehensive test infrastructure
- ✅ **Benchmarking Suite**: Performance comparison with Node.js V8
- ✅ **Visualization**: Interactive HTML dashboard for results
- 🔧 **Pool Optimization**: Pre-allocation strategy (see POOL_OPTIMIZATION.md)

## Development Workflow

1. **Write Code**: Implement features in `src/` directory
2. **Write Tests**: Create corresponding test files in `tests/`
3. **Build**: Use `./scripts/build` to compile
4. **Test**: Run `./scripts/test` to validate changes
5. **Sync**: Use `./scripts/sync` to update from remote repository

## Benchmarking

The project includes a comprehensive benchmarking suite that compares performance with Node.js V8:

### Running Benchmarks

```bash
# Run full benchmark suite
./benchmarks/scripts/run_benchmark.sh

# View results in browser
open benchmarks/visualize.html
```

### Benchmark Features

- **Performance Metrics**: Parse time, throughput (MB/s)
- **Memory Tracking**: Heap usage, pool efficiency, RSS monitoring
- **Comparison**: Side-by-side with Node.js V8 parser
- **Historical Data**: Track performance across commits
- **Visualization**: Interactive charts showing:
  - Throughput comparison
  - Memory usage breakdown (heap + pool)
  - Pool efficiency statistics
  - Performance progression over time

### Sample Results

```
large_array.json (10K elements):
  Parse time: 2.56 ms
  Throughput: 457 MB/s
  Memory: Peak heap 77 MB + Pool 62 MB = ~139 MB total
  vs Node.js: 130 MB RSS
```

### Benchmark Data Location

- **Latest Results**: `benchmarks/results/latest/`
- **Historical Results**: `benchmarks/results/history/`
- **Test Files**: `benchmarks/data/`

## Performance Optimizations

### Current Optimizations

- **Memory Pool**: ~2x faster allocation than malloc/free
- **String Interning**: Reduced string duplication
- **Cache-Friendly Layout**: Contiguous memory allocation

### Planned Optimizations

- **Array Pre-allocation**: Two-pass counting to eliminate resizing waste (see POOL_OPTIMIZATION.md)
- **SIMD String Parsing**: Vectorized string processing
- **JIT Token Caching**: Reduce re-tokenization overhead

See `PERFORMANCE_ANALYSIS.md` for detailed performance analysis.

## Future Enhancements

- JSON serialization (JSON to string)
- Pretty printing with indentation
- Validation and schema checking
- Unicode support improvements
- Streaming parser for large files

## Contributing

1. Ensure all tests pass: `./scripts/test`
2. Add tests for new features
3. Follow existing code style and naming conventions
4. Update documentation as needed

## Examples

Check the `samples/` directory for example JSON files:

- `simple.json`: Basic object with mixed types
- `array.json`: Array examples
- `nested.json`: Complex nested structures

For benchmark test files, see `benchmarks/data/`:
- `large.json`: 13.3 MB test file
- `xlarge.json`: 37.9 MB test file
- `large_array.json`: 10K element array
- `deeply_nested.json`: Deep nesting test
- And more...

## License

This project is open source. See the license file for details.

## Documentation

- **[POOL_OPTIMIZATION.md](POOL_OPTIMIZATION.md)**: Guide to implementing array pre-allocation optimization
- **[PERFORMANCE_ANALYSIS.md](PERFORMANCE_ANALYSIS.md)**: Detailed performance analysis of two-pass approach
- **[OPTIMIZATION.md](OPTIMIZATION.md)**: Memory optimization strategies and findings

## Performance Highlights

- **Competitive with Node.js V8**: Similar throughput on most workloads
- **Memory Efficient**: Pool-based allocation reduces fragmentation
- **Fast Parsing**: ~450-600 MB/s on typical JSON files
- **Low Overhead**: Minimal memory waste with pool allocator

---

*This JSON parser is designed for educational purposes and demonstrates clean C programming practices for lexical analysis, parsing, and memory management.*
