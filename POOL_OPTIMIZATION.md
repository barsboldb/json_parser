# Implementation Guide: Pre-allocation for Pool-based JSON Arrays

## Overview

Replace dynamic array resizing with a two-pass approach:
1. **Pass 1**: Count elements by skipping tokens (no allocation)
2. **Pass 2**: Allocate exact size, parse directly into array

**Goal**: Eliminate dead weight in memory pool from geometric growth (1→2→4→8→16...)

---

## Architecture

### Current Flow (Wasteful)
```
parse_array()
  └─> Create array with capacity 0
      └─> For each element:
          ├─> parse_value()
          └─> json_array_push_pooled()
              └─> if len >= cap * 0.75:
                  ├─> Allocate new array (2x size)
                  ├─> Copy old → new
                  └─> OLD ARRAY = DEAD WEIGHT ❌
```

### New Flow (Zero Waste)
```
parse_array()
  ├─> Pass 1: count_array_elements()
  │   ├─> Save lexer position
  │   ├─> Skip through values (fast)
  │   ├─> Count elements
  │   └─> Restore lexer position
  │
  └─> Pass 2: Allocate & Parse
      ├─> Allocate array with EXACT count
      └─> Parse values directly into array (no resizing!)
```

---

## Implementation Steps

### Step 1: Implement Value Skipping

**File**: `src/parser.c`

```c
// Skip a single JSON value without allocating memory
static void skip_value(parser_t *parser) {
  token_type_t type = parser->current_token.type;

  switch (type) {
    case TOKEN_LBRACKET:
      advance(parser);
      skip_array(parser);
      break;

    case TOKEN_LBRACE:
      advance(parser);
      skip_object(parser);
      break;

    // Primitives - just skip
    case TOKEN_STRING:
    case TOKEN_NUMBER:
    case TOKEN_TRUE:
    case TOKEN_FALSE:
    case TOKEN_NULL:
      advance(parser);
      break;

    default:
      parser_error(parser, "Unexpected token while skipping");
      advance(parser);  // Try to recover
  }
}
```

**Common Pitfalls:**
- ❌ Don't allocate ANY memory during skip
- ❌ Don't parse string/number values
- ✅ Just advance tokens
- ✅ Handle errors gracefully

---

### Step 2: Implement Array Skipping

```c
static void skip_array(parser_t *parser) {
  // Already consumed '[', now skip to ']'

  // Empty array: []
  if (check(parser, TOKEN_RBRACKET)) {
    advance(parser);
    return;
  }

  // Skip elements
  while (true) {
    skip_value(parser);

    if (check(parser, TOKEN_COMMA)) {
      advance(parser);
      continue;
    } else if (check(parser, TOKEN_RBRACKET)) {
      advance(parser);
      break;
    } else {
      parser_error(parser, "Expected ',' or ']' while skipping array");
      return;
    }
  }
}
```

**Common Pitfalls:**
- ❌ Don't forget empty array case `[]`
- ❌ Don't forget to advance past `]`
- ✅ Match exact same logic as parse_array()
- ✅ Handle malformed JSON gracefully

---

### Step 3: Implement Object Skipping

```c
static void skip_object(parser_t *parser) {
  // Already consumed '{', now skip to '}'

  // Empty object: {}
  if (check(parser, TOKEN_RBRACE)) {
    advance(parser);
    return;
  }

  // Skip key-value pairs
  while (true) {
    // Skip key (must be string)
    if (!check(parser, TOKEN_STRING)) {
      parser_error(parser, "Expected string key while skipping object");
      return;
    }
    advance(parser);

    // Skip colon
    if (!check(parser, TOKEN_COLON)) {
      parser_error(parser, "Expected ':' while skipping object");
      return;
    }
    advance(parser);

    // Skip value
    skip_value(parser);

    if (check(parser, TOKEN_COMMA)) {
      advance(parser);
      continue;
    } else if (check(parser, TOKEN_RBRACE)) {
      advance(parser);
      break;
    } else {
      parser_error(parser, "Expected ',' or '}' while skipping object");
      return;
    }
  }
}
```

**Common Pitfalls:**
- ❌ Don't forget empty object case `{}`
- ❌ Don't forget colon between key:value
- ✅ Keys must be strings (JSON spec)
- ✅ Mirror parse_object() logic exactly

---

### Step 4: Implement Element Counting

```c
static size_t count_array_elements(parser_t *parser) {
  // CRITICAL: Save lexer state
  size_t saved_position = parser->lexer->current;
  token_t saved_token = parser->current_token;
  bool saved_error = parser->has_error;

  size_t count = 0;

  // Empty array?
  if (check(parser, TOKEN_RBRACKET)) {
    return 0;
  }

  // Count elements by skipping
  while (true) {
    skip_value(parser);

    // Stop on error
    if (parser->has_error) {
      count = 0;  // Fallback to dynamic growth on error
      break;
    }

    count++;

    if (check(parser, TOKEN_COMMA)) {
      advance(parser);
      continue;
    } else if (check(parser, TOKEN_RBRACKET)) {
      break;
    } else {
      // Malformed - fallback
      count = 0;
      break;
    }
  }

  // CRITICAL: Restore lexer state
  parser->lexer->current = saved_position;
  parser->current_token = saved_token;
  parser->has_error = saved_error;

  return count;
}
```

**Common Pitfalls:**
- ❌ **CRITICAL**: Must save/restore ALL parser state
  - lexer position
  - current token
  - error state
  - any other stateful fields
- ❌ Don't forget to handle errors during counting
- ✅ On error, return 0 to fallback to dynamic growth
- ✅ Test with nested arrays: `[[1,2],[3,4]]`

---

### Step 5: Update parse_array()

```c
json_value_t parse_array(parser_t *parser) {
  if (!check(parser, TOKEN_LBRACKET)) {
    parser_error(parser, "Expected '['");
    return json_value_array_pooled(0, parser->pool);
  }
  advance(parser);  // Consume '['

  // Check for empty array
  if (check(parser, TOKEN_RBRACKET)) {
    advance(parser);
    return json_value_array_pooled(0, parser->pool);
  }

  // NEW: Count elements first
  size_t count = count_array_elements(parser);

  // Allocate array with exact size
  json_value_t array = json_value_array_pooled(count, parser->pool);

  // If count failed (error during counting), fallback to dynamic growth
  if (count == 0) {
    // Use old logic: parse with push_pooled
    while (true) {
      json_value_t element = parse_value(parser);
      if (parser->has_error) return array;

      json_array_push_pooled(&array, element, parser->pool);

      if (check(parser, TOKEN_COMMA)) {
        advance(parser);
      } else if (check(parser, TOKEN_RBRACKET)) {
        break;
      } else {
        parser_error(parser, "Expected ',' or ']'");
        return array;
      }
    }
  } else {
    // NEW: Parse directly into pre-allocated array
    for (size_t i = 0; i < count; i++) {
      array.array.items[i] = parse_value(parser);
      array.array.len = i + 1;  // Update length as we go

      if (parser->has_error) {
        advance(parser);  // consume ']'
        return array;
      }

      // Expect comma or end
      if (i < count - 1) {
        if (!check(parser, TOKEN_COMMA)) {
          parser_error(parser, "Expected ','");
          return array;
        }
        advance(parser);
      }
    }
  }

  if (!check(parser, TOKEN_RBRACKET)) {
    parser_error(parser, "Expected ']'");
    return array;
  }
  advance(parser);

  return array;
}
```

**Common Pitfalls:**
- ❌ Don't forget fallback to old logic if counting fails
- ❌ Don't forget to update `array.len` as you parse
- ❌ Count-1 elements have comma, last element doesn't
- ✅ Handle parse errors mid-array
- ✅ Test with: empty `[]`, single `[1]`, multiple `[1,2,3]`

---

### Step 6: Update json_value_array_pooled()

**File**: `src/json.c`

```c
json_value_t json_value_array_pooled(size_t initial_capacity, mem_pool_t *pool) {
  json_value_t value;
  value.type = JSON_ARRAY;

  // Handle zero capacity
  if (initial_capacity == 0) {
    initial_capacity = 1;  // Minimum allocation
  }

  value.array.items = pool_alloc(pool, initial_capacity * sizeof(json_value_t));
  value.array.len = 0;
  value.array.cap = initial_capacity;
  value.is_pooled = true;

  return value;
}
```

**Common Pitfalls:**
- ❌ Don't allow zero-sized allocations
- ✅ Set `is_pooled = true`
- ✅ Initialize len = 0, cap = initial_capacity

---

## Optional: Apply Same Optimization to Objects

The same problem exists with `json_object_insert_pooled()`. You can apply the same strategy:

```c
static size_t count_object_keys(parser_t *parser) {
  // Save state
  size_t saved_position = parser->lexer->current;
  token_t saved_token = parser->current_token;
  bool saved_error = parser->has_error;

  size_t count = 0;

  if (check(parser, TOKEN_RBRACE)) {
    return 0;
  }

  while (true) {
    if (!check(parser, TOKEN_STRING)) break;
    advance(parser);  // key

    if (!check(parser, TOKEN_COLON)) break;
    advance(parser);  // colon

    skip_value(parser);  // value
    count++;

    if (check(parser, TOKEN_COMMA)) {
      advance(parser);
    } else if (check(parser, TOKEN_RBRACE)) {
      break;
    } else {
      count = 0;
      break;
    }
  }

  // Restore state
  parser->lexer->current = saved_position;
  parser->current_token = saved_token;
  parser->has_error = saved_error;

  return count;
}
```

---

## Testing Strategy

### Test Cases (in order of complexity)

1. **Empty array**: `[]`
2. **Single element**: `[1]`
3. **Multiple primitives**: `[1, 2, 3, 4, 5]`
4. **Nested arrays**: `[[1, 2], [3, 4]]`
5. **Mixed types**: `[1, "hello", true, null]`
6. **Deep nesting**: `[[[[[1]]]]]`
7. **Large array**: `[1, 2, 3, ..., 10000]`
8. **Malformed**: `[1, 2,]` (trailing comma)
9. **Object in array**: `[{"a": 1}, {"b": 2}]`
10. **Array in object**: `{"items": [1, 2, 3]}`

### Verification

```c
// Add debug output during development
printf("Array count: %zu, allocated: %zu, used: %zu\n",
       count, array.array.cap, array.array.len);

// After parsing
assert(array.array.len == array.array.cap);  // Should be exact!
```

### Create Test File

```json
// test_prealloc.json
{
  "empty": [],
  "single": [1],
  "multiple": [1, 2, 3, 4, 5],
  "nested": [[1, 2], [3, 4]],
  "mixed": [1, "hello", true, null, {"x": 10}],
  "large": [/* generate 1000 elements */]
}
```

---

## Performance Verification

### Before (with resizing waste)
```
large_array.json (10K elements):
  Pool allocated: 9.4 MB
  Pool used: 8.8 MB
  Waste: ~600 KB (6%)

  But actual waste from dead arrays:
  1 + 2 + 4 + 8 + ... + 8192 = ~16K items wasted
```

### After (exact allocation)
```
large_array.json (10K elements):
  Pool allocated: 8.8 MB
  Pool used: 8.8 MB
  Waste: 0 KB (0%) ✨

  Allocations reduced: 14 → 1 per array!
```

### Run Benchmark

```bash
./benchmarks/scripts/run_benchmark.sh
```

Check the visualization at `benchmarks/visualize.html`:
- Pool efficiency should be ~100%
- Total allocations should be much lower
- Pool allocated ≈ Pool used

---

## Common Pitfalls Summary

### Critical Mistakes to Avoid

1. **❌ Not restoring parser state**
   ```c
   // WRONG: Counting consumed tokens, now parse_value fails!
   count_array_elements(parser);  // Moves position
   parse_value(parser);  // Expects to be at element, but we're at ']'
   ```

2. **❌ Forgetting to handle nested structures**
   ```c
   // WRONG: Only skips flat values
   skip_value(parser);  // What if value is [1, [2, 3]]?
   ```

3. **❌ Not handling errors during counting**
   ```c
   // WRONG: Malformed JSON during count, but we still allocate!
   size_t count = count_array_elements(parser);  // Returns wrong count
   allocate(count);  // Allocate wrong size!
   ```

4. **❌ Off-by-one with commas**
   ```c
   // WRONG: Expecting comma after last element
   for (i = 0; i < count; i++) {
     parse_value();
     advance_comma();  // ❌ No comma after last element!
   }
   ```

5. **❌ Not testing with empty structures**
   ```json
   []        // Empty array
   {}        // Empty object
   [[], {}]  // Array with empty children
   ```

6. **❌ Token duplication issues**
   ```c
   // If you copy tokens instead of just position:
   saved_token = parser->current_token;  // Does token have allocated memory?
   // Later restore might cause double-free or memory leak
   ```

---

## Debugging Tips

### Add Temporary Logging

```c
#ifdef DEBUG_POOL_ALLOC
  printf("[COUNT] Array has %zu elements\n", count);
  printf("[ALLOC] Allocating capacity %zu\n", array.array.cap);
  printf("[PARSE] Parsed %zu/%zu elements\n", i+1, count);
#endif
```

Enable with:
```bash
gcc -DDEBUG_POOL_ALLOC ...
```

### Check Invariants

```c
// After parsing complete
assert(array.array.len <= array.array.cap);
assert(array.array.len == count);  // Should match!

// If using pool
assert(array.is_pooled == true);
```

### Visual Pool Inspection

Add to benchmark output:
```c
printf("Pool efficiency: %.1f%%\n",
       100.0 * pool_used / pool_allocated);
// Should be ~100% after optimization!
```

### Memory Leak Detection

```bash
# Run with valgrind
valgrind --leak-check=full ./bin/parser test.json

# Check for:
# - "definitely lost" (should be 0)
# - "still reachable" (pool memory, expected)
```

---

## Alternative: Chunked Arrays (Advanced)

If you want to explore the chunked approach instead:

```c
#define CHUNK_SIZE 64

typedef struct array_chunk {
  json_value_t items[CHUNK_SIZE];
  size_t count;
  struct array_chunk *next;
} array_chunk_t;

// Approach:
// 1. Parse into linked list of chunks (no resizing)
// 2. Count total elements
// 3. Allocate final array
// 4. Flatten chunks into array
// 5. Free chunks (keep final array)

// Pro: No resizing ever, no two-pass needed
// Con: Need final copy to contiguous array
// Con: More complex code
```

---

## Expected Results

### Memory Savings
- **Small arrays** (< 64 items): ~20-30% pool reduction
- **Medium arrays** (64-1K items): ~40-50% pool reduction
- **Large arrays** (10K+ items): ~60-70% pool reduction

### Allocation Reduction
- **Before**: ~log₂(N) allocations per array
- **After**: 1 allocation per array
- **Example**: 10K array: 14 allocations → 1

### Benchmark Diff
```diff
  large_array.json:
-   Pool allocated: 9.4 MB
+   Pool allocated: 8.8 MB
-   Pool used: 8.8 MB
+   Pool used: 8.8 MB
-   Efficiency: 93%
+   Efficiency: 100%
-   Allocations: 1100
+   Allocations: 100
```

---

## Implementation Checklist

- [ ] Step 1: Implement `skip_value()`
- [ ] Step 2: Implement `skip_array()`
- [ ] Step 3: Implement `skip_object()`
- [ ] Step 4: Implement `count_array_elements()`
- [ ] Step 5: Update `parse_array()`
- [ ] Step 6: Update `json_value_array_pooled()`
- [ ] Test: Empty array `[]`
- [ ] Test: Single element `[1]`
- [ ] Test: Multiple elements `[1,2,3]`
- [ ] Test: Nested arrays `[[1,2],[3,4]]`
- [ ] Test: Large array (10K+ elements)
- [ ] Test: Malformed JSON (error handling)
- [ ] Verify: Pool efficiency ≈ 100%
- [ ] Verify: Allocation count reduced
- [ ] Optional: Apply to objects too
- [ ] Benchmark: Run full benchmark suite
- [ ] Verify: No memory leaks (valgrind)

---

## Resources

### Relevant Files
- `src/parser.c` - Main parsing logic
- `src/json.c` - JSON value operations
- `include/parser.h` - Parser interface
- `include/mem_pool.h` - Memory pool interface
- `benchmarks/src/bench_parser.c` - Benchmark runner

### Useful Commands
```bash
# Build with debug symbols
make clean && make debug

# Run single test
./bin/parser test.json

# Run benchmarks
./benchmarks/scripts/run_benchmark.sh

# Check for leaks
valgrind --leak-check=full ./bin/parser test.json

# View results
open benchmarks/visualize.html
```

---

Good luck with implementation! 🚀
