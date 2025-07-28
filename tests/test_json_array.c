#include "../include/json.h"
#include "test_framework.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TEST_SUITE_INIT()

void test_json_value_array_creation() {
  printf("\n=== Testing json_value_array creation ===\n");

  // Test 1: Create array with initial capacity
  json_value_t *arr = json_value_array(4);
  TEST_ASSERT(arr != NULL, "Array creation should not return NULL");
  TEST_ASSERT(arr->type == JSON_ARRAY, "Array should have JSON_ARRAY type");
  TEST_ASSERT(arr->array.len == 0, "New array should have length 0");
  TEST_ASSERT(arr->array.cap == 4, "New array should have capacity 4");
  TEST_ASSERT(arr->array.items != NULL, "Array items should not be NULL");
  json_value_free(arr);

  // Test 2: Create array with different initial capacity
  json_value_t *arr2 = json_value_array(10);
  TEST_ASSERT(arr2 != NULL, "Second array creation should not return NULL");
  TEST_ASSERT(arr2->array.cap == 10, "Array should have capacity 10");
  TEST_ASSERT(arr2->array.len == 0, "Second array should start with length 0");
  json_value_free(arr2);

  // Test 3: Create array with zero capacity (edge case)
  json_value_t *arr3 = json_value_array(0);
  TEST_ASSERT(arr3 != NULL, "Array with zero capacity should still be created");
  TEST_ASSERT(arr3->array.cap == 0, "Array should have capacity 0");
  TEST_ASSERT(arr3->array.len == 0, "Array should have length 0");
  json_value_free(arr3);
}

void test_json_array_push() {
  printf("\n=== Testing json_array_push ===\n");

  json_value_t *arr = json_value_array(2);

  // Test 1: Push string value
  json_value_t *str_val = json_value_string("hello");
  json_array_push(arr, str_val);
  TEST_ASSERT(arr->array.len == 1, "Array length should be 1 after first push");
  TEST_ASSERT(arr->array.items[0] == str_val,
              "First item should be the pushed string");
  TEST_ASSERT(arr->array.items[0]->type == JSON_STRING,
              "First item should be string type");

  // Test 2: Push number value
  json_value_t *num_val = json_value_number(42.5);
  json_array_push(arr, num_val);
  TEST_ASSERT(arr->array.len == 2,
              "Array length should be 2 after second push");
  TEST_ASSERT(arr->array.items[1] == num_val,
              "Second item should be the pushed number");
  TEST_ASSERT(arr->array.items[1]->type == JSON_NUMBER,
              "Second item should be number type");

  // Test 3: Push boolean value (should trigger resize)
  json_value_t *bool_val = json_value_bool(true);
  json_array_push(arr, bool_val);
  TEST_ASSERT(arr->array.len == 3, "Array length should be 3 after third push");
  TEST_ASSERT(arr->array.cap >= 3, "Array capacity should accommodate 3 items");
  TEST_ASSERT(arr->array.items[2] == bool_val,
              "Third item should be the pushed boolean");

  // Test 4: Push null value
  json_value_t *null_val = json_value_init(JSON_NULL);
  json_array_push(arr, null_val);
  TEST_ASSERT(arr->array.len == 4,
              "Array length should be 4 after fourth push");
  TEST_ASSERT(arr->array.items[3] == null_val,
              "Fourth item should be the pushed null");
  TEST_ASSERT(arr->array.items[3]->type == JSON_NULL,
              "Fourth item should be null type");

  // Verify all items are still accessible
  TEST_ASSERT(strcmp(arr->array.items[0]->string, "hello") == 0,
              "String value should be preserved");
  TEST_ASSERT(arr->array.items[1]->number == 42.5,
              "Number value should be preserved");
  TEST_ASSERT(arr->array.items[2]->boolean == true,
              "Boolean value should be preserved");

  json_value_free(arr);
}

void test_json_array_pop() {
  printf("\n=== Testing json_array_pop ===\n");

  json_value_t *arr = json_value_array(8);

  // Test popping from empty array (edge case)
  json_value_t *empty_pop = json_array_pop(arr);
  TEST_ASSERT(empty_pop == NULL, "Popping from empty array should return NULL");
  TEST_ASSERT(arr->array.len == 0, "Array length should remain 0");

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

  // Test 1: Pop last item (LIFO behavior)
  json_value_t *popped = json_array_pop(arr);
  TEST_ASSERT(popped == val4,
              "Popped item should be the last pushed item (null)");
  TEST_ASSERT(arr->array.len == 3, "Array length should be 3 after first pop");
  TEST_ASSERT(popped->type == JSON_NULL,
              "Popped value should maintain its type");
  json_value_free(popped);

  // Test 2: Pop another item
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val3, "Popped item should be the boolean");
  TEST_ASSERT(arr->array.len == 2, "Array length should be 2 after second pop");
  TEST_ASSERT(popped->boolean == false, "Boolean value should be preserved");
  json_value_free(popped);

  // Test 3: Pop third item
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val2, "Popped item should be the number");
  TEST_ASSERT(arr->array.len == 1, "Array length should be 1 after third pop");
  TEST_ASSERT(popped->number == 123, "Number value should be preserved");
  json_value_free(popped);

  // Test 4: Pop final item
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val1, "Popped item should be the string");
  TEST_ASSERT(arr->array.len == 0, "Array should be empty after fourth pop");
  TEST_ASSERT(strcmp(popped->string, "first") == 0,
              "String value should be preserved");
  json_value_free(popped);

  // Test 5: Pop from empty array again
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == NULL, "Popping from empty array should return NULL");

  json_value_free(arr);
}

void test_json_array_mixed_operations() {
  printf("\n=== Testing mixed array operations ===\n");

  json_value_t *arr = json_value_array(2);

  // Test push and pop sequence
  json_value_t *val1 = json_value_string("test1");
  json_value_t *val2 = json_value_number(456.789);

  json_array_push(arr, val1);
  json_array_push(arr, val2);
  TEST_ASSERT(arr->array.len == 2, "Array should have 2 items after pushes");

  json_value_t *popped = json_array_pop(arr);
  TEST_ASSERT(popped == val2, "Popped item should be val2 (LIFO)");
  TEST_ASSERT(arr->array.len == 1, "Array should have 1 item after pop");
  TEST_ASSERT(popped->number == 456.789,
              "Popped number value should be correct");
  json_value_free(popped);

  // Push more items
  json_value_t *val3 = json_value_bool(true);
  json_value_t *val4 = json_value_init(JSON_NULL);
  json_array_push(arr, val3);
  json_array_push(arr, val4);
  TEST_ASSERT(arr->array.len == 3,
              "Array should have 3 items after more pushes");

  // Verify remaining items are in correct order
  TEST_ASSERT(arr->array.items[0] == val1, "First item should still be val1");
  TEST_ASSERT(arr->array.items[1] == val3, "Second item should be val3");
  TEST_ASSERT(arr->array.items[2] == val4, "Third item should be val4");

  // Pop all remaining items and verify LIFO order
  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val4, "First pop should return val4 (last pushed)");
  json_value_free(popped);

  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val3, "Second pop should return val3");
  json_value_free(popped);

  popped = json_array_pop(arr);
  TEST_ASSERT(popped == val1, "Third pop should return val1 (first pushed)");
  json_value_free(popped);

  TEST_ASSERT(arr->array.len == 0, "Array should be empty after all pops");

  json_value_free(arr);
}

void test_json_array_capacity_management() {
  printf("\n=== Testing array capacity management ===\n");

  json_value_t *arr = json_value_array(2);
  size_t initial_cap = arr->array.cap;

  // Fill array to trigger expansion
  json_value_t *values[10];
  for (int i = 0; i < 10; i++) {
    values[i] = json_value_number(i * 10.5);
    json_array_push(arr, values[i]);
  }

  TEST_ASSERT(arr->array.len == 10, "Array should have 10 items");
  TEST_ASSERT(arr->array.cap >= 10,
              "Array capacity should accommodate all items");
  TEST_ASSERT(arr->array.cap > initial_cap,
              "Array capacity should have expanded");

  // Verify all values are preserved during capacity changes
  for (int i = 0; i < 10; i++) {
    TEST_ASSERT(arr->array.items[i] == values[i],
                "Array item should be preserved during expansion");
    TEST_ASSERT(arr->array.items[i]->number == i * 10.5,
                "Value should be preserved during expansion");
  }

  // Pop items to potentially trigger contraction
  size_t expanded_cap = arr->array.cap;
  for (int i = 0; i < 8; i++) {
    json_value_t *popped = json_array_pop(arr);
    json_value_free(popped);
  }

  TEST_ASSERT(arr->array.len == 2, "Array should have 2 items after pops");

  // Verify remaining items are still correct
  TEST_ASSERT(arr->array.items[0]->number == 0.0,
              "First remaining item should be correct");
  TEST_ASSERT(arr->array.items[1]->number == 10.5,
              "Second remaining item should be correct");

  json_value_free(arr);
}

void test_json_array_edge_cases() {
  printf("\n=== Testing array edge cases ===\n");

  // Test with NULL values
  json_value_t *arr = json_value_array(4);

  // Push NULL pointer (should handle gracefully)
  // Note: This depends on your implementation - some might assert, others might
  // handle gracefully json_array_push(arr, NULL);  // Uncomment if your
  // implementation handles this

  // Test with nested arrays
  json_value_t *nested_arr = json_value_array(2);
  json_value_t *inner_val = json_value_string("nested");
  json_array_push(nested_arr, inner_val);
  json_array_push(arr, nested_arr);

  TEST_ASSERT(arr->array.len == 1, "Array should contain the nested array");
  TEST_ASSERT(arr->array.items[0] == nested_arr,
              "First item should be the nested array");
  TEST_ASSERT(arr->array.items[0]->type == JSON_ARRAY,
              "Nested item should be array type");

  json_value_free(arr);
}

// Use the TEST_MAIN macro for cleaner main function
TEST_MAIN("JSON Array",
          test_json_value_array_creation();
          test_json_array_push(); test_json_array_pop();
          test_json_array_mixed_operations();
          test_json_array_capacity_management();
          test_json_array_edge_cases();
          )
