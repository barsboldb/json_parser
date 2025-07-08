#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../include/json.h"

static int tests_run = 0;
static int tests_passed = 0;

#define GREEN "\033[0;32m"
#define RED   "\033[0;31m"
#define RESET "\033[0m"

#define TEST_ASSERT(condition, message) \
  do { \
    tests_run++; \
    if (condition) { \
      tests_passed++; \
      printf(GREEN "✓ %s\n" RESET, message); \
    } else { \
      printf(RED "✗ %s\n" RESET, message); \
    } \
  } while (0)

void test_json_value_array_creation() {
  printf("\n=== Testing json_value_array creation ===\n");

  // Test 1: Create array with initial capacity;
  json_value_t *arr = json_value_array(4);
  TEST_ASSERT(arr != NULL, "Array creation should not return NULL");
  TEST_ASSERT(arr->type == JSON_ARRAY, "Array should have JSON_ARRAY type");
  TEST_ASSERT(arr->array.len == 0, "New array should have length 0");
  TEST_ASSERT(arr->array.cap == 4, "New array should have capacity 4");
  TEST_ASSERT(arr->array.items != NULL, "Array items should not be NULL");
  
  json_value_free(arr);

  // Test 2: Create array with different initial capacity;
  json_value_t *arr2 = json_value_array(10);
  TEST_ASSERT(arr->array.cap == 10, "Array should have capacity 10");

  json_value_free(arr2);
}

void test_json_array_push() {
  printf("\n=== Testing json_array_push ===\n");

  json_value_t *arr = json_value_array(2);

  // Test 1: Push string value
  json_value_t *str_val = json_value_string("hello");
  json_array_push(arr, str_val);
  TEST_ASSERT(arr->array.len == 1, "Array length should be 1 after first push");
  TEST_ASSERT(arr->array.items[0] == str_val, "First item should be the pushed string");

  // Test 2: Push number value
  json_value_t *num_val = json_value_number(42.5);
  json_array_push(arr, num_val);
  TEST_ASSERT(arr->array.len == 2, "Array length should be 2 after second push");
  TEST_ASSERT(arr->array.items[1] == num_val, "Second items should be the pushed number");

  // Test 3: Push boolean value (should trigger resize)
  json_value_t *bool_val = json_value_bool(true);
  json_array_push(arr, bool_val);
  TEST_ASSERT(arr->array.len == 3, "Array length should be 3 after third push");
  TEST_ASSERT(arr->array.cap == 4, "Array capacity should double to 4");
  TEST_ASSERT(arr->array.items[2] == bool_val, "Third item should be the pushed boolean");

  // Test 4: Push null value (should trigger another resize)
  json_value_t *null_val = json_value_init(JSON_NULL);
  json_array_push(arr, null_val);
  TEST_ASSERT(arr->array.len == 4, "Array length should be 4 after fourth push");
  TEST_ASSERT(arr->array.cap == 8, "Array capacity should double to 8");
  TEST_ASSERT(arr->array.items[3], "Fourth item should be the pushed null");

  json_value_free(arr);
}

void test_json_array_pop() {
  printf("\n=== Testing json_array_pop ===\n");

  json_value_t *arr = json_value_array(8);

  // Fill array with test values
  json_value_t *val1 = json_value_string("first");
  json_value_t *val2 = json_value_number(123);
  json_value_t *val3 = json_value_bool(false);
  json_value_t *val4 = json_value_init(JSON_NULL);

  json_array_push(arr, val1);
  json_array_push(arr, val2);
  json_array_push(arr, val3);
  json_array_push(arr, val4);

  TEST_ASSERT(arr->array.len == 4, "Array should have 4 items before popping");

  // Test 1: Pop last item
  json_value_t *popped = json_array_pop(arr);
  TEST_ASSERT(popped == val4, "Popped item should be the last pushed item (null)");
  TEST_ASSERT(arr->array.len == 3, "Array length should be 3 after first pop");
  json_value_free(popped);

  // Test 2: Pop another item
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val3, "Popped item should be the boolean");
  TEST_ASSERT(arr->array.len == 2, "Array length should be 2 after second pop");
  json_value_free(popped);

  // Test 3: Pop yet another item (should trigger resize)
  popped = json_array_pop(arr);
  printf("%p %p\n", val2, popped);
  TEST_ASSERT(popped == val2, "Popped item should be the number");
  TEST_ASSERT(arr->array.len == 1, "Array length should be 1 after third pop");
  TEST_ASSERT(arr->array.cap == 4, "Array capacity should halve to 4");
  /* json_value_free(popped); */

  // Test 4: Pop yet another item
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val1, "Popped item should be the string");
  TEST_ASSERT(arr->array.len == 0, "Array should be empty after fourth pop");
  TEST_ASSERT(arr->array.cap == 2, "Array capacity should halve to 2");

  json_value_free(arr);
}

void test_json_array_mixed_operation() {
  printf("\n=== Testing mixed array operations ===\n");

  json_value_t *arr = json_value_array(2);
  
  // Test push and pop sequence
  json_value_t *val1 = json_value_string("test1");
  json_value_t *val2 = json_value_number(456);
  
  json_array_push(arr, val1);
  json_array_push(arr, val2);
  TEST_ASSERT(arr->array.len == 2, "Array should have 2 items");
  
  json_value_t *popped = json_array_pop(arr);
  TEST_ASSERT(popped == val2, "Popped item should be val2");
  TEST_ASSERT(arr->array.len == 1, "Array should have 1 item after pop");
  
  // Push more items
  json_value_t *val3 = json_value_bool(true);
  json_value_t *val4 = json_value_init(JSON_NULL);
  json_array_push(arr, val3);
  json_array_push(arr, val4);
  TEST_ASSERT(arr->array.len == 3, "Array should have 3 items");
  
  // Pop all items and verify order
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val4, "First pop should return val4");
  
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val3, "Second pop should return val3");
  
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val1, "Third pop should return val1");
  
  TEST_ASSERT(arr->array.len == 0, "Array should be empty after all pops");
  
  // Cleanup
  json_value_free(arr);
}

void test_json_array_capacity_management() {
  printf("\n=== Testing array capacity management ===\n");
  
  json_value_t *arr = json_value_array(4);
  
  // Fill to trigger expansion
  for (int i = 0; i < 10; i++) {
      json_value_t *val = json_value_number(i);
      json_array_push(arr, val);
  }
  
  TEST_ASSERT(arr->array.len == 10, "Array should have 10 items");
  TEST_ASSERT(arr->array.cap == 16, "Array capacity should be 16 after expansions");
  
  // Pop to trigger contraction
  for (int i = 0; i < 8; i++) {
      json_array_pop(arr);
  }
  
  TEST_ASSERT(arr->array.len == 2, "Array should have 2 items after pops");
  TEST_ASSERT(arr->array.cap == 8, "Array capacity should contract to 8");
  
  // Cleanup
  json_value_free(arr);
}

void run_all_tests() {
  printf("Running JSON Array Tests...\n");
  printf("==============================\n");
  
  test_json_value_array_creation();
  test_json_array_push();
  test_json_array_pop();
  test_json_array_mixed_operation();
  test_json_array_capacity_management();

  printf("\n==============================\n");
  printf("Test Results: %d/%d tests passed\n", tests_passed, tests_run);

  if (tests_passed == tests_run) {
    printf("🎉 All tests passed!\n");
  } else {
    printf("❌ Some tests failed!\n");
  }
}

int main() {
  run_all_tests();
  return (tests_passed == tests_run) ? 0 : 1;
}
