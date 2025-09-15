# JSON Parser in C

A lightweight JSON lexer and parser implementation written in C from scratch. This project provides a complete tokenization system for JSON data with proper error handling and memory management.

## Features

- **Complete JSON Lexical Analysis**: Tokenizes all JSON data types including objects, arrays, strings, numbers, booleans, and null values
- **Robust Number Parsing**: Handles integers, floats, scientific notation, and negative numbers according to JSON specification
- **String Escape Handling**: Properly processes escape sequences in JSON strings
- **Error Reporting**: Provides line and column information for debugging
- **Memory Safe**: Proper memory allocation and deallocation for all tokens
- **Standards Compliant**: Follows JSON specification for parsing rules
- **Comprehensive Testing**: Built-in test framework for reliable development

## Project Structure

```
.
├── include/
│   ├── json.h          # JSON value structures and types
│   └── lexer.h         # Lexer interface and token definitions
├── src/
│   ├── main.c          # Example usage and testing
│   ├── lexer.c         # Lexer implementation
│   ├── json.c          # JSON parser (to be implemented)
│   └── parser.c        # Parser implementation (to be implemented)
├── tests/
│   ├── test_framework.h # Testing framework header
│   └── test_*.c        # Individual test files
├── samples/
│   ├── simple.json     # Basic JSON example
│   ├── array.json      # Array examples
│   └── nested.json     # Complex nested structures
├── scripts/
│   ├── build           # Build script
│   ├── test            # Test runner script
│   └── sync            # Git synchronization script
└── dist/               # Compiled binaries (auto-generated)
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

### Lexer Functions

#### `lexer_t lexer_init(const char *input)`
Initializes a lexer with the given JSON input string.

#### `token_t next_token(lexer_t *lexer)`
Returns the next token from the input stream.

#### `void token_free(token_t *token)`
Frees memory allocated for a token's lexeme.

#### `void print_token(token_t *token)`
Prints token information for debugging.

### Utility Functions

#### `int is_digit(char ch)`
Checks if a character is a digit.

#### `int is_space(char ch)`
Checks if a character is whitespace.

#### `void skip_whitespace(lexer_t *lexer)`
Advances the lexer past whitespace characters.

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

- All tokens allocate memory for their lexeme strings
- **Important**: Always call `token_free()` for each token to prevent memory leaks
- The lexer creates a copy of the input string for safe processing

## Development Status

- ✅ **Lexer**: Complete with full JSON tokenization
- ✅ **Build System**: Automated build and test scripts
- ✅ **Testing Framework**: Comprehensive test infrastructure
- ⏳ **Parser**: Structure defined, implementation pending
- ⏳ **JSON Values**: Data structures ready, parsing logic needed

## Development Workflow

1. **Write Code**: Implement features in `src/` directory
2. **Write Tests**: Create corresponding test files in `tests/`
3. **Build**: Use `./scripts/build` to compile
4. **Test**: Run `./scripts/test` to validate changes
5. **Sync**: Use `./scripts/sync` to update from remote repository

## Future Enhancements

- Complete parser implementation for building JSON AST
- JSON serialization (JSON to string)
- Pretty printing with indentation
- Validation and schema checking
- Performance optimizations
- Unicode support improvements
- Benchmarking suite

## Contributing

1. Ensure all tests pass: `./scripts/test`
2. Add tests for new features
3. Follow existing code style and naming conventions
4. Update documentation as needed

## Examples

Check the `samples/` directory for example JSON files:

- `simple.json`: Basic object with mixed types
- `array.json`: Array examples (to be added)
- `nested.json`: Complex nested structures (to be added)

## License

This project is open source. See the license file for details.

---

*This JSON parser is designed for educational purposes and demonstrates clean C programming practices for lexical analysis and parsing.*
