#include "../include/json.h"
#include "test_framework.h"

TEST_SUITE_INIT()

void test_json_array_stress() {
  printf("\n=== Testing array stress scenarios ===\n");

  json_value_t arr = json_value_array(1);

  // Test 1: Push many items to test multiple resizes
  const int num_items = 100;
  for (int i = 0; i < num_items; i++) {
    json_value_t val = json_value_number(i);
    json_array_push(&arr, val);
  }

  TEST_ASSERT(arr.array.len == num_items, "Array should contain all pushed items");
  TEST_ASSERT(arr.array.cap >= num_items, "Array capacity should accommodate all items");

  // Verify all values are correct
  for (int i = 0; i < num_items; i++) {
    TEST_ASSERT(arr.array.items[i].type == JSON_NUMBER, "Item should be number type");
    TEST_ASSERT(arr.array.items[i].number == i, "Item value should be correct");
  }

  // Test 2: Pop all items
  for (int i = num_items - 1; i >= 0; i--) {
    int pop_result = json_array_pop(&arr);
    TEST_ASSERT(pop_result == 0, "Pop should succeed");
    TEST_ASSERT(arr.array.len == i, "Array length should decrease correctly");
  }

  TEST_ASSERT(arr.array.len == 0, "Array should be empty after all pops");

  // Test 3: Pop from empty array after stress test
  int final_pop = json_array_pop(&arr);
  TEST_ASSERT(final_pop == -1, "Final pop from empty array should return -1");

  free(arr.array.items);
}

void test_json_array_memory_management() {
  printf("\n=== Testing array memory management ===\n");

  // Test creating and destroying many arrays
  for (int i = 0; i < 10; i++) {
    json_value_t arr = json_value_array(i + 1);
    
    // Add some items
    for (int j = 0; j < i; j++) {
      json_value_t val = json_value_number(j * i);
      json_array_push(&arr, val);
    }
    
    TEST_ASSERT(arr.array.len == i, "Array should have correct number of items");
    
    // Pop half the items
    for (int j = 0; j < i / 2; j++) {
      json_array_pop(&arr);
    }
    
    TEST_ASSERT(arr.array.len == i - (i / 2), "Array should have correct length after pops");
    
    free(arr.array.items);
  }
  
  TEST_ASSERT(1, "Memory management test completed without crashes");
}

TEST_MAIN("JSON Array Stress",
  test_json_array_stress();
  test_json_array_memory_management();
)
