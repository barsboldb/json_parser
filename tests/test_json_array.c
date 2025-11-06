#include "../include/json.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TEST_SUITE_INIT()

void test_json_value_array_creation() {
  printf("\n=== Testing json_value_array creation ===\n");

  // Test 1: Create array with initial capacity
  json_value_t arr = json_value_array(4);
  TEST_ASSERT(arr.type == JSON_ARRAY, "Array should have JSON_ARRAY type");
  TEST_ASSERT(arr.array.len == 0, "New array should have length 0");
  TEST_ASSERT(arr.array.cap == 4, "New array should have capacity 4");
  TEST_ASSERT(arr.array.items != NULL, "Array items should not be NULL");
  free(arr.array.items);

  // Test 2: Create array with different initial capacity
  json_value_t arr2 = json_value_array(10);
  TEST_ASSERT(arr2.type == JSON_ARRAY, "Second array should have JSON_ARRAY type");
  TEST_ASSERT(arr2.array.cap == 10, "Array should have capacity 10");
  TEST_ASSERT(arr2.array.len == 0, "Second array should start with length 0");
  free(arr2.array.items);

  // Test 3: Create array with zero capacity (uses minimum capacity)
  json_value_t arr3 = json_value_array(0);
  TEST_ASSERT(arr3.type == JSON_ARRAY, "Array with zero capacity should have JSON_ARRAY type");
  TEST_ASSERT(arr3.array.cap == ARRAY_MIN_CAP, "Array should have minimum capacity");
  TEST_ASSERT(arr3.array.len == 0, "Array should have length 0");
  TEST_ASSERT(arr3.array.items != NULL, "Array items should be allocated");
  free(arr3.array.items);
}

void test_json_array_push() {
  printf("\n=== Testing json_array_push ===\n");

  json_value_t arr = json_value_array(2);

  // Test 1: Push string value
  json_value_t str_val = json_value_string("hello");
  json_array_push(&arr, str_val);
  TEST_ASSERT(arr.array.len == 1, "Array length should be 1 after first push");
  TEST_ASSERT(arr.array.items[0].type == JSON_STRING, "First item should be string type");
  TEST_ASSERT(strcmp(arr.array.items[0].string, "hello") == 0, "String value should be correct");

  // Test 2: Push number value
  json_value_t num_val = json_value_number(42.5);
  json_array_push(&arr, num_val);
  TEST_ASSERT(arr.array.len == 2, "Array length should be 2 after second push");
  TEST_ASSERT(arr.array.items[1].type == JSON_NUMBER, "Second item should be number type");
  TEST_ASSERT(arr.array.items[1].number == 42.5, "Number value should be correct");

  // Test 3: Push boolean value (should trigger resize since we're at 75% capacity)
  json_value_t bool_val = json_value_bool(true);
  json_array_push(&arr, bool_val);
  TEST_ASSERT(arr.array.len == 3, "Array length should be 3 after third push");
  TEST_ASSERT(arr.array.cap >= 3, "Array capacity should accommodate 3 items");
  TEST_ASSERT(arr.array.items[2].type == JSON_BOOL, "Third item should be boolean type");
  TEST_ASSERT(arr.array.items[2].boolean == true, "Boolean value should be correct");

  // Test 4: Push null value
  json_value_t null_val = json_value_init(JSON_NULL);
  json_array_push(&arr, null_val);
  TEST_ASSERT(arr.array.len == 4, "Array length should be 4 after fourth push");
  TEST_ASSERT(arr.array.items[3].type == JSON_NULL, "Fourth item should be null type");

  // Verify all items are still accessible
  TEST_ASSERT(strcmp(arr.array.items[0].string, "hello") == 0, "String value should be preserved");
  TEST_ASSERT(arr.array.items[1].number == 42.5, "Number value should be preserved");
  TEST_ASSERT(arr.array.items[2].boolean == true, "Boolean value should be preserved");

  // Clean up
  /* free(arr.array.items[0].string); */
  /* free(arr.array.items); */
}

void test_json_array_pop() {
  printf("\n=== Testing json_array_pop ===\n");

  json_value_t arr = json_value_array(8);

  // Test popping from empty array (edge case)
  int empty_result = json_array_pop(&arr);
  TEST_ASSERT(empty_result == -1, "Popping from empty array should return -1");
  TEST_ASSERT(arr.array.len == 0, "Array length should remain 0");

  // Fill array with test values
  json_value_t val1 = json_value_string("first");
  json_value_t val2 = json_value_number(123);
  json_value_t val3 = json_value_bool(false);
  json_value_t val4 = json_value_init(JSON_NULL);

  json_array_push(&arr, val1);
  json_array_push(&arr, val2);
  json_array_push(&arr, val3);
  json_array_push(&arr, val4);

  TEST_ASSERT(arr.array.len == 4, "Array should have 4 items before popping");

  // Test 1: Pop last item (LIFO behavior)
  int pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == 0, "Pop should return 0 on success");
  TEST_ASSERT(arr.array.len == 3, "Array length should be 3 after first pop");

  // Test 2: Pop another item
  pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == 0, "Second pop should return 0 on success");
  TEST_ASSERT(arr.array.len == 2, "Array length should be 2 after second pop");

  // Test 3: Pop third item
  pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == 0, "Third pop should return 0 on success");
  TEST_ASSERT(arr.array.len == 1, "Array length should be 1 after third pop");

  // Test 4: Pop final item
  pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == 0, "Fourth pop should return 0 on success");
  TEST_ASSERT(arr.array.len == 0, "Array should be empty after fourth pop");

  // Test 5: Pop from empty array again
  pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == -1, "Popping from empty array should return -1");

  // Clean up
  /* free(val1.string); */
  /* free(arr.array.items); */
}

void test_json_array_mixed_operations() {
  printf("\n=== Testing mixed array operations ===\n");

  json_value_t arr = json_value_array(2);

  // Test push and pop sequence
  json_value_t val1 = json_value_string("test1");
  json_value_t val2 = json_value_number(456.789);

  json_array_push(&arr, val1);
  json_array_push(&arr, val2);
  TEST_ASSERT(arr.array.len == 2, "Array should have 2 items after pushes");

  int pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == 0, "Pop should succeed");
  TEST_ASSERT(arr.array.len == 1, "Array should have 1 item after pop");

  // The last item should be val1 (LIFO - val2 was popped)
  TEST_ASSERT(arr.array.items[0].type == JSON_STRING, "Remaining item should be string");
  TEST_ASSERT(strcmp(arr.array.items[0].string, "test1") == 0, "Remaining string should be correct");

  // Push more items
  json_value_t val3 = json_value_bool(true);
  json_value_t val4 = json_value_init(JSON_NULL);
  json_array_push(&arr, val3);
  json_array_push(&arr, val4);
  TEST_ASSERT(arr.array.len == 3, "Array should have 3 items after more pushes");

  // Verify items are in correct order
  TEST_ASSERT(arr.array.items[0].type == JSON_STRING, "First item should be string");
  TEST_ASSERT(arr.array.items[1].type == JSON_BOOL, "Second item should be boolean");
  TEST_ASSERT(arr.array.items[2].type == JSON_NULL, "Third item should be null");

  // Pop all remaining items and verify LIFO order
  pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == 0, "Pop should succeed");
  TEST_ASSERT(arr.array.len == 2, "Array should have 2 items after pop");

  pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == 0, "Pop should succeed");
  TEST_ASSERT(arr.array.len == 1, "Array should have 1 item after pop");

  pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == 0, "Pop should succeed");
  TEST_ASSERT(arr.array.len == 0, "Array should be empty after all pops");

  // Clean up
  free(val1.string);
  free(arr.array.items);
}

void test_json_array_capacity_management() {
  printf("\n=== Testing array capacity management ===\n");

  json_value_t arr = json_value_array(2);
  size_t initial_cap = arr.array.cap;

  // Fill array to trigger expansion
  json_value_t values[10];
  for (int i = 0; i < 10; i++) {
    values[i] = json_value_number(i * 10.5);
    json_array_push(&arr, values[i]);
  }

  TEST_ASSERT(arr.array.len == 10, "Array should have 10 items");
  TEST_ASSERT(arr.array.cap >= 10, "Array capacity should accommodate all items");
  TEST_ASSERT(arr.array.cap > initial_cap, "Array capacity should have expanded");

  // Verify all values are preserved during capacity changes
  for (int i = 0; i < 10; i++) {
    TEST_ASSERT(arr.array.items[i].type == JSON_NUMBER, "Array item should preserve type during expansion");
    TEST_ASSERT(arr.array.items[i].number == i * 10.5, "Value should be preserved during expansion");
  }

  // Pop items to potentially trigger contraction
  for (int i = 0; i < 8; i++) {
    int pop_result = json_array_pop(&arr);
    TEST_ASSERT(pop_result == 0, "Pop should succeed");
  }

  TEST_ASSERT(arr.array.len == 2, "Array should have 2 items after pops");

  // Verify remaining items are still correct
  TEST_ASSERT(arr.array.items[0].number == 0.0, "First remaining item should be correct");
  TEST_ASSERT(arr.array.items[1].number == 10.5, "Second remaining item should be correct");

  free(arr.array.items);
}

void test_json_array_edge_cases() {
  printf("\n=== Testing array edge cases ===\n");

  json_value_t arr = json_value_array(4);

  // Test with nested arrays
  json_value_t nested_arr = json_value_array(2);
  json_value_t inner_val = json_value_string("nested");
  json_array_push(&nested_arr, inner_val);
  json_array_push(&arr, nested_arr);

  TEST_ASSERT(arr.array.len == 1, "Array should contain the nested array");
  TEST_ASSERT(arr.array.items[0].type == JSON_ARRAY, "Nested item should be array type");
  TEST_ASSERT(arr.array.items[0].array.len == 1, "Nested array should have correct length");
  TEST_ASSERT(arr.array.items[0].array.items[0].type == JSON_STRING, "Nested string should have correct type");
  TEST_ASSERT(strcmp(arr.array.items[0].array.items[0].string, "nested") == 0, "Nested string should have correct value");

  // Test pop from array with nested structures
  int pop_result = json_array_pop(&arr);
  TEST_ASSERT(pop_result == 0, "Pop should succeed");
  TEST_ASSERT(arr.array.len == 0, "Array should be empty after pop");

  // Clean up
  free(inner_val.string);
  free(nested_arr.array.items);
  free(arr.array.items);
}

void test_json_array_comparison() {
  printf("\n=== Testing json_array_cmp ===\n");

  // Test 1: Compare arrays with different lengths
  json_value_t arr1 = json_value_array(2);
  json_value_t arr2 = json_value_array(3);
  
  json_value_t val1 = json_value_number(1);
  json_array_push(&arr1, val1);
  
  json_value_t val2 = json_value_number(1);
  json_value_t val3 = json_value_number(2);
  json_array_push(&arr2, val2);
  json_array_push(&arr2, val3);
  
  TEST_ASSERT(json_array_cmp(&arr1, &arr2) == -1, "Arrays with different lengths should return -1");
  TEST_ASSERT(json_array_cmp(&arr2, &arr1) == -1, "Arrays with different lengths should return -1 (reversed)");

  // Test 2: Compare empty arrays
  json_value_t empty1 = json_value_array(0);
  json_value_t empty2 = json_value_array(0);
  TEST_ASSERT(json_array_cmp(&empty1, &empty2) == 0, "Empty arrays should be equal");

  // Test 3: Compare arrays with same elements
  json_value_t arr3 = json_value_array(2);
  json_value_t val4 = json_value_number(10);
  json_value_t val5 = json_value_string("test");
  json_array_push(&arr3, val4);
  json_array_push(&arr3, val5);
  
  json_value_t arr4 = json_value_array(2);
  json_value_t val6 = json_value_number(10);
  json_value_t val7 = json_value_string("test");
  json_array_push(&arr4, val6);
  json_array_push(&arr4, val7);
  
  TEST_ASSERT(json_array_cmp(&arr3, &arr4) == 0, "Arrays with same elements should be equal");

  // Test 4: Compare arrays with different elements
  json_value_t arr5 = json_value_array(1);
  json_value_t val8 = json_value_string("different");
  json_array_push(&arr5, val8);
  
  json_value_t arr6 = json_value_array(1);
  json_value_t val9 = json_value_string("test");
  json_array_push(&arr6, val9);
  
  int cmp_result = json_array_cmp(&arr5, &arr6);
  TEST_ASSERT(cmp_result != 0, "Arrays with different elements should not be equal");

  // Test 5: Compare non-array types
  json_value_t not_array = json_value_number(42);
  TEST_ASSERT(json_array_cmp(&arr1, &not_array) == -1, "Array vs non-array should return -1");
  TEST_ASSERT(json_array_cmp(&not_array, &arr1) == -1, "Non-array vs array should return -1");

  // Clean up
  free(arr1.array.items);
  free(arr2.array.items);
  free(arr3.array.items);
  free(arr4.array.items);
  free(arr5.array.items);
  free(arr6.array.items);
  free(val5.string);
  free(val7.string);
  free(val8.string);
  free(val9.string);
}

void test_json_array_null_safety() {
  printf("\n=== Testing array null safety ===\n");

  // Test pop with NULL pointer
  int null_pop_result = json_array_pop(NULL);
  TEST_ASSERT(null_pop_result == -1, "Pop with NULL array should return -1");

  // Test pop with non-array type
  json_value_t not_array = json_value_number(42);
  int non_array_pop = json_array_pop(&not_array);
  TEST_ASSERT(non_array_pop == -1, "Pop with non-array type should return -1");

  // Test array comparison with NULL
  json_value_t arr = json_value_array(1);
  TEST_ASSERT(json_array_cmp(NULL, &arr) == -1, "Comparison with NULL should return -1");
  TEST_ASSERT(json_array_cmp(&arr, NULL) == -1, "Comparison with NULL should return -1");

  free(arr.array.items);
}

TEST_MAIN("JSON Array",
  test_json_value_array_creation();
  test_json_array_push();
  test_json_array_pop();
  test_json_array_mixed_operations();
  test_json_array_comparison();
  test_json_array_capacity_management();
  test_json_array_null_safety();
)
