#include "../include/json.h"
#include "test_framework.h"
#include <math.h>

TEST_SUITE_INIT()

void test_json_value_init() {
  printf("\n=== Testing json_value_init ===\n");

  // Test 1: Initialize JSON_NULL
  json_value_t null_val = json_value_init(JSON_NULL);
  TEST_ASSERT(null_val.type == JSON_NULL, "Null value should have JSON_NULL type");

  // Test 2: Initialize JSON_NUMBER
  json_value_t num_val = json_value_init(JSON_NUMBER);
  TEST_ASSERT(num_val.type == JSON_NUMBER, "Number value should have JSON_NUMBER type");

  // Test 3: Initialize JSON_BOOL
  json_value_t bool_val = json_value_init(JSON_BOOL);
  TEST_ASSERT(bool_val.type == JSON_BOOL, "Boolean value should have JSON_BOOL type");

  // Test 4: Initialize JSON_STRING
  json_value_t str_val = json_value_init(JSON_STRING);
  TEST_ASSERT(str_val.type == JSON_STRING, "String value should have JSON_STRING type");

  // Test 5: Initialize JSON_ARRAY
  json_value_t arr_val = json_value_init(JSON_ARRAY);
  TEST_ASSERT(arr_val.type == JSON_ARRAY, "Array value should have JSON_ARRAY type");

  // Test 6: Initialize JSON_OBJECT
  json_value_t obj_val = json_value_init(JSON_OBJECT);
  TEST_ASSERT(obj_val.type == JSON_OBJECT, "Object value should have JSON_OBJECT type");
}

void test_json_value_string() {
  printf("\n=== Testing json_value_string ===\n");

  // Test 1: Create string value with regular text
  json_value_t str_val = json_value_string("hello world");
  TEST_ASSERT(str_val.type == JSON_STRING, "String value should have JSON_STRING type");
  TEST_ASSERT(str_val.string != NULL, "String pointer should not be NULL");
  TEST_ASSERT(strcmp(str_val.string, "hello world") == 0, "String content should match input");

  // Test 2: Create string value with empty string
  json_value_t empty_str = json_value_string("");
  TEST_ASSERT(empty_str.type == JSON_STRING, "Empty string should have JSON_STRING type");
  TEST_ASSERT(empty_str.string != NULL, "Empty string pointer should not be NULL");
  TEST_ASSERT(strlen(empty_str.string) == 0, "Empty string should have zero length");
  TEST_ASSERT(strcmp(empty_str.string, "") == 0, "Empty string content should be empty");

  // Test 3: Create string with special characters
  json_value_t special_str = json_value_string("Hello\nWorld\t!");
  TEST_ASSERT(special_str.type == JSON_STRING, "Special string should have JSON_STRING type");
  TEST_ASSERT(strcmp(special_str.string, "Hello\nWorld\t!") == 0, "Special characters should be preserved");

  // Test 4: Create string with quotes
  json_value_t quoted_str = json_value_string("\"quoted\"");
  TEST_ASSERT(quoted_str.type == JSON_STRING, "Quoted string should have JSON_STRING type");
  TEST_ASSERT(strcmp(quoted_str.string, "\"quoted\"") == 0, "Quotes should be preserved in string");

  // Test 5: Create long string
  char long_string[1000];
  for (int i = 0; i < 999; i++) {
    long_string[i] = 'A' + (i % 26);
  }
  long_string[999] = '\0';
  
  json_value_t long_str = json_value_string(long_string);
  TEST_ASSERT(long_str.type == JSON_STRING, "Long string should have JSON_STRING type");
  TEST_ASSERT(strcmp(long_str.string, long_string) == 0, "Long string content should match");
  TEST_ASSERT(strlen(long_str.string) == 999, "Long string should have correct length");

  // Clean up allocated strings
  free(str_val.string);
  free(empty_str.string);
  free(special_str.string);
  free(quoted_str.string);
  free(long_str.string);
}

void test_json_value_number() {
  printf("\n=== Testing json_value_number ===\n");

  // Test 1: Create positive integer
  json_value_t pos_int = json_value_number(42);
  TEST_ASSERT(pos_int.type == JSON_NUMBER, "Positive integer should have JSON_NUMBER type");
  TEST_ASSERT(pos_int.number == 42.0, "Positive integer value should be correct");

  // Test 2: Create negative integer
  json_value_t neg_int = json_value_number(-17);
  TEST_ASSERT(neg_int.type == JSON_NUMBER, "Negative integer should have JSON_NUMBER type");
  TEST_ASSERT(neg_int.number == -17.0, "Negative integer value should be correct");

  // Test 3: Create zero
  json_value_t zero = json_value_number(0);
  TEST_ASSERT(zero.type == JSON_NUMBER, "Zero should have JSON_NUMBER type");
  TEST_ASSERT(zero.number == 0.0, "Zero value should be correct");

  // Test 4: Create positive decimal
  json_value_t pos_decimal = json_value_number(3.14159);
  TEST_ASSERT(pos_decimal.type == JSON_NUMBER, "Positive decimal should have JSON_NUMBER type");
  TEST_ASSERT(fabs(pos_decimal.number - 3.14159) < 0.00001, "Positive decimal value should be correct");

  // Test 5: Create negative decimal
  json_value_t neg_decimal = json_value_number(-2.71828);
  TEST_ASSERT(neg_decimal.type == JSON_NUMBER, "Negative decimal should have JSON_NUMBER type");
  TEST_ASSERT(fabs(neg_decimal.number - (-2.71828)) < 0.00001, "Negative decimal value should be correct");

  // Test 6: Create very large number
  json_value_t large_num = json_value_number(1e10);
  TEST_ASSERT(large_num.type == JSON_NUMBER, "Large number should have JSON_NUMBER type");
  TEST_ASSERT(large_num.number == 1e10, "Large number value should be correct");

  // Test 7: Create very small number
  json_value_t small_num = json_value_number(1e-10);
  TEST_ASSERT(small_num.type == JSON_NUMBER, "Small number should have JSON_NUMBER type");
  TEST_ASSERT(fabs(small_num.number - 1e-10) < 1e-15, "Small number value should be correct");
}

void test_json_value_bool() {
  printf("\n=== Testing json_value_bool ===\n");

  // Test 1: Create true boolean
  json_value_t true_val = json_value_bool(true);
  TEST_ASSERT(true_val.type == JSON_BOOL, "True value should have JSON_BOOL type");
  TEST_ASSERT(true_val.boolean == true, "True value should be true");

  // Test 2: Create false boolean
  json_value_t false_val = json_value_bool(false);
  TEST_ASSERT(false_val.type == JSON_BOOL, "False value should have JSON_BOOL type");
  TEST_ASSERT(false_val.boolean == false, "False value should be false");

  // Test 3: Create boolean from non-zero integer (should be true)
  json_value_t nonzero_bool = json_value_bool(42);
  TEST_ASSERT(nonzero_bool.type == JSON_BOOL, "Non-zero boolean should have JSON_BOOL type");
  TEST_ASSERT(nonzero_bool.boolean == true, "Non-zero value should convert to true");

  // Test 4: Create boolean from zero integer (should be false)
  json_value_t zero_bool = json_value_bool(0);
  TEST_ASSERT(zero_bool.type == JSON_BOOL, "Zero boolean should have JSON_BOOL type");
  TEST_ASSERT(zero_bool.boolean == false, "Zero value should convert to false");
}

void test_json_value_array() {
  printf("\n=== Testing json_value_array ===\n");

  // Test 1: Create array with initial capacity
  json_value_t arr = json_value_array(5);
  TEST_ASSERT(arr.type == JSON_ARRAY, "Array should have JSON_ARRAY type");
  TEST_ASSERT(arr.array.items != NULL, "Array items should not be NULL");
  TEST_ASSERT(arr.array.len == 0, "New array should have length 0");
  TEST_ASSERT(arr.array.cap == 5, "Array should have specified capacity");

  // Test 2: Create array with zero capacity
  json_value_t empty_arr = json_value_array(0);
  TEST_ASSERT(empty_arr.type == JSON_ARRAY, "Empty array should have JSON_ARRAY type");
  TEST_ASSERT(empty_arr.array.len == 0, "Empty array should have length 0");
  TEST_ASSERT(empty_arr.array.cap == 0, "Empty array should have capacity 0");

  // Test 3: Create array with large capacity
  json_value_t large_arr = json_value_array(1000);
  TEST_ASSERT(large_arr.type == JSON_ARRAY, "Large array should have JSON_ARRAY type");
  TEST_ASSERT(large_arr.array.cap == 1000, "Large array should have correct capacity");
  TEST_ASSERT(large_arr.array.len == 0, "Large array should start with length 0");

  // Clean up allocated arrays
  free(arr.array.items);
  free(large_arr.array.items);
}

void test_json_value_object() {
  printf("\n=== Testing json_value_object ===\n");

  // Test 1: Create object with initial capacity
  json_value_t obj = json_value_object(3);
  TEST_ASSERT(obj.type == JSON_OBJECT, "Object should have JSON_OBJECT type");
  TEST_ASSERT(obj.object.entries != NULL, "Object entries should not be NULL");
  TEST_ASSERT(obj.object.len == 0, "New object should have length 0");
  TEST_ASSERT(obj.object.cap == 3, "Object should have specified capacity");

  // Test 2: Create object with zero capacity
  json_value_t empty_obj = json_value_object(0);
  TEST_ASSERT(empty_obj.type == JSON_OBJECT, "Empty object should have JSON_OBJECT type");
  TEST_ASSERT(empty_obj.object.len == 0, "Empty object should have length 0");
  TEST_ASSERT(empty_obj.object.cap == 0, "Empty object should have capacity 0");

  // Test 3: Create object with large capacity
  json_value_t large_obj = json_value_object(100);
  TEST_ASSERT(large_obj.type == JSON_OBJECT, "Large object should have JSON_OBJECT type");
  TEST_ASSERT(large_obj.object.cap == 100, "Large object should have correct capacity");
  TEST_ASSERT(large_obj.object.len == 0, "Large object should start with length 0");

  // Clean up allocated objects
  free(obj.object.entries);
  free(large_obj.object.entries);
}

void test_json_value_cmp() {
  printf("\n=== Testing json_value_cmp ===\n");

  // Test 1: Compare different types (should return -1)
  json_value_t str_val = json_value_string("test");
  json_value_t num_val = json_value_number(42);
  TEST_ASSERT(json_value_cmp(&str_val, &num_val) == -1, "Different types should return -1");
  TEST_ASSERT(json_value_cmp(&num_val, &str_val) == -1, "Different types should return -1 (reversed)");

  // Test 2: Compare null values
  json_value_t null1 = json_value_init(JSON_NULL);
  json_value_t null2 = json_value_init(JSON_NULL);
  TEST_ASSERT(json_value_cmp(&null1, &null2) == 0, "Null values should be equal");

  // Test 3: Compare number values
  json_value_t num1 = json_value_number(10);
  json_value_t num2 = json_value_number(20);
  json_value_t num3 = json_value_number(10);
  TEST_ASSERT(json_value_cmp(&num1, &num2) == -10, "10 - 20 should be -10");
  TEST_ASSERT(json_value_cmp(&num2, &num1) == 10, "20 - 10 should be 10");
  TEST_ASSERT(json_value_cmp(&num1, &num3) == 0, "Equal numbers should return 0");

  // Test 4: Compare boolean values
  json_value_t bool_true1 = json_value_bool(true);
  json_value_t bool_true2 = json_value_bool(true);
  json_value_t bool_false = json_value_bool(false);
  TEST_ASSERT(json_value_cmp(&bool_true1, &bool_true2) == 0, "Equal booleans should return 0");
  TEST_ASSERT(json_value_cmp(&bool_true1, &bool_false) == 1, "true - false should be 1");
  TEST_ASSERT(json_value_cmp(&bool_false, &bool_true1) == -1, "false - true should be -1");

  // Test 5: Compare string values
  json_value_t str1 = json_value_string("apple");
  json_value_t str2 = json_value_string("banana");
  json_value_t str3 = json_value_string("apple");
  TEST_ASSERT(json_value_cmp(&str1, &str2) < 0, "\"apple\" should come before \"banana\"");
  TEST_ASSERT(json_value_cmp(&str2, &str1) > 0, "\"banana\" should come after \"apple\"");
  TEST_ASSERT(json_value_cmp(&str1, &str3) == 0, "Equal strings should return 0");

  // Clean up allocated strings
  free(str_val.string);
  free(str1.string);
  free(str2.string);
  free(str3.string);
}

void test_json_array_cmp() {
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

  // Clean up
  free(arr1.array.items);
  free(arr2.array.items);
  free(arr3.array.items);
  free(arr4.array.items);
  free(val5.string);
  free(val7.string);
}

void test_json_object_cmp() {
  printf("\n=== Testing json_object_cmp ===\n");

  // Test 1: Compare objects with different lengths
  json_value_t obj1 = json_value_object(1);
  json_value_t obj2 = json_value_object(2);
  obj1.object.len = 1;
  obj2.object.len = 2;
  
  TEST_ASSERT(json_object_cmp(&obj1, &obj2) == -1, "Objects with different lengths should return -1");

  // Test 2: Compare empty objects
  json_value_t empty_obj1 = json_value_object(0);
  json_value_t empty_obj2 = json_value_object(0);
  TEST_ASSERT(json_object_cmp(&empty_obj1, &empty_obj2) == 0, "Empty objects should be equal");

  // Test 3: Compare non-object types
  json_value_t not_obj = json_value_number(42);
  json_value_t obj = json_value_object(1);
  TEST_ASSERT(json_object_cmp(&not_obj, &obj) == -1, "Non-object compared to object should return -1");

  // Clean up
  free(obj1.object.entries);
  free(obj2.object.entries);
  free(obj.object.entries);
}

void test_json_value_free() {
  printf("\n=== Testing json_value_free ===\n");

  // Test 1: Free string value
  json_value_t *str_val = malloc(sizeof(json_value_t));
  *str_val = json_value_string("test string");
  // Note: json_value_free should free both the string content and the value itself
  // This test mainly ensures no crashes occur
  json_value_free(str_val);
  TEST_ASSERT(1, "String value freed without crash");

  // Test 2: Free array value
  json_value_t *arr_val = malloc(sizeof(json_value_t));
  *arr_val = json_value_array(5);
  json_value_free(arr_val);
  TEST_ASSERT(1, "Array value freed without crash");

  // Test 3: Free number value (no dynamic allocation)
  json_value_t *num_val = malloc(sizeof(json_value_t));
  *num_val = json_value_number(123.45);
  json_value_free(num_val);
  TEST_ASSERT(1, "Number value freed without crash");

  // Test 4: Free boolean value (no dynamic allocation)
  json_value_t *bool_val = malloc(sizeof(json_value_t));
  *bool_val = json_value_bool(true);
  json_value_free(bool_val);
  TEST_ASSERT(1, "Boolean value freed without crash");

  // Test 5: Free null value (no dynamic allocation)
  json_value_t *null_val = malloc(sizeof(json_value_t));
  *null_val = json_value_init(JSON_NULL);
  json_value_free(null_val);
  TEST_ASSERT(1, "Null value freed without crash");
}

void test_json_value_edge_cases() {
  printf("\n=== Testing json_value edge cases ===\n");

  // Test 1: String with null character in middle
  char str_with_null[] = {'h', 'e', 'l', '\0', 'l', 'o', '\0'};
  json_value_t str_val = json_value_string(str_with_null);
  TEST_ASSERT(str_val.type == JSON_STRING, "String with embedded null should have correct type");
  TEST_ASSERT(strlen(str_val.string) == 3, "String should stop at first null character");
  free(str_val.string);

  // Test 2: Very large number
  json_value_t large_num = json_value_number(1.7976931348623157e+308);
  TEST_ASSERT(large_num.type == JSON_NUMBER, "Very large number should have correct type");
  TEST_ASSERT(large_num.number == 1.7976931348623157e+308, "Very large number should preserve value");

  // Test 3: Very small number (close to zero)
  json_value_t tiny_num = json_value_number(2.2250738585072014e-308);
  TEST_ASSERT(tiny_num.type == JSON_NUMBER, "Very small number should have correct type");
  TEST_ASSERT(tiny_num.number > 0, "Very small positive number should remain positive");

  // Test 4: Negative zero
  json_value_t neg_zero = json_value_number(-0.0);
  TEST_ASSERT(neg_zero.type == JSON_NUMBER, "Negative zero should have correct type");
  TEST_ASSERT(neg_zero.number == 0.0, "Negative zero should equal positive zero");

  // Test 5: Compare NaN values (if your implementation supports them)
  // Note: This might not be relevant for JSON which doesn't support NaN
  // json_value_t nan_val = json_value_number(NAN);
  // TEST_ASSERT(isnan(nan_val.number), "NaN value should remain NaN");
}

TEST_MAIN("JSON Value Test", 
  test_json_value_init();
  test_json_value_string();
  test_json_value_number();
  test_json_value_bool();
  test_json_value_array();
  test_json_value_object();
  test_json_value_cmp();
  test_json_array_cmp();
  test_json_object_cmp();
  test_json_value_free();
  test_json_value_edge_cases();
)
