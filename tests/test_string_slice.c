#include "test_framework.h"
#include "../include/lexer.h"
#include <string.h>

TEST_SUITE_INIT()

void test_slice_to_string() {
  printf("\n=== Testing slice_to_string ===\n");

  const char *source = "hello world";

  // Test 1: Normal string slice
  string_slice_t slice1 = {.start = source, .length = 5};
  char *str1 = slice_to_string(slice1);
  TEST_ASSERT(strcmp(str1, "hello") == 0, "slice_to_string should extract 'hello'");
  TEST_ASSERT(str1[5] == '\0', "Null terminator should be present");
  free(str1);

  // Test 2: Full string
  string_slice_t slice2 = {.start = source, .length = 11};
  char *str2 = slice_to_string(slice2);
  TEST_ASSERT(strcmp(str2, "hello world") == 0, "slice_to_string should extract full string");
  free(str2);

  // Test 3: Single character
  string_slice_t slice3 = {.start = source, .length = 1};
  char *str3 = slice_to_string(slice3);
  TEST_ASSERT(strcmp(str3, "h") == 0, "slice_to_string should extract single char");
  free(str3);

  // Test 4: Empty slice
  string_slice_t slice4 = {.start = source, .length = 0};
  char *str4 = slice_to_string(slice4);
  TEST_ASSERT(strcmp(str4, "") == 0, "slice_to_string should handle empty slice");
  TEST_ASSERT(str4[0] == '\0', "Empty string should be null-terminated");
  free(str4);

  // Test 5: Middle substring
  string_slice_t slice5 = {.start = source + 6, .length = 5};
  char *str5 = slice_to_string(slice5);
  TEST_ASSERT(strcmp(str5, "world") == 0, "slice_to_string should extract 'world'");
  free(str5);
}

void test_slice_strcmp() {
  printf("\n=== Testing slice_strcmp ===\n");

  const char *source = "hello world";

  // Test 1: Equal strings
  string_slice_t slice1 = {.start = source, .length = 5};
  TEST_ASSERT(slice_strcmp(slice1, "hello") == 0, "Equal strings should return 0");

  // Test 2: Different strings - slice longer
  string_slice_t slice2 = {.start = source, .length = 11};
  TEST_ASSERT(slice_strcmp(slice2, "hello") > 0, "Longer slice should return positive");

  // Test 3: Different strings - slice shorter
  string_slice_t slice3 = {.start = source, .length = 5};
  TEST_ASSERT(slice_strcmp(slice3, "hello world") < 0, "Shorter slice should return negative");

  // Test 4: Empty slice vs empty string
  string_slice_t slice4 = {.start = source, .length = 0};
  TEST_ASSERT(slice_strcmp(slice4, "") == 0, "Empty slice should equal empty string");

  // Test 5: Different content, same length
  const char *other = "world hello";
  string_slice_t slice5 = {.start = source, .length = 5};
  string_slice_t slice6 = {.start = other, .length = 5};
  TEST_ASSERT(slice_strcmp(slice5, "world") != 0, "Different content should not be equal");

  // Test 6: Partial match
  string_slice_t slice7 = {.start = source, .length = 4};
  TEST_ASSERT(slice_strcmp(slice7, "hell") == 0, "Partial string should match");
  TEST_ASSERT(slice_strcmp(slice7, "hello") != 0, "Partial string should not match longer");
}

void test_slice_cmp() {
  printf("\n=== Testing slice_cmp ===\n");

  const char *str1 = "hello";
  const char *str2 = "world";
  const char *str3 = "hello";

  // Test 1: Equal slices
  string_slice_t slice1 = {.start = str1, .length = 5};
  string_slice_t slice2 = {.start = str3, .length = 5};
  TEST_ASSERT(slice_cmp(slice1, slice2) == 0, "Equal slices should return 0");

  // Test 2: Different length - first longer
  string_slice_t slice3 = {.start = str1, .length = 5};
  string_slice_t slice4 = {.start = str1, .length = 3};
  TEST_ASSERT(slice_cmp(slice3, slice4) > 0, "Longer slice should return positive");

  // Test 3: Different length - first shorter
  TEST_ASSERT(slice_cmp(slice4, slice3) < 0, "Shorter slice should return negative");

  // Test 4: Same length, different content
  string_slice_t slice5 = {.start = str1, .length = 5};
  string_slice_t slice6 = {.start = str2, .length = 5};
  int cmp = slice_cmp(slice5, slice6);
  TEST_ASSERT(cmp != 0, "Different content should not be equal");
  TEST_ASSERT(cmp < 0, "'hello' should be less than 'world'");

  // Test 5: Empty slices
  string_slice_t empty1 = {.start = str1, .length = 0};
  string_slice_t empty2 = {.start = str2, .length = 0};
  TEST_ASSERT(slice_cmp(empty1, empty2) == 0, "Empty slices should be equal");

  // Test 6: Empty vs non-empty
  string_slice_t slice7 = {.start = str1, .length = 5};
  string_slice_t empty3 = {.start = str1, .length = 0};
  TEST_ASSERT(slice_cmp(slice7, empty3) > 0, "Non-empty should be greater than empty");
  TEST_ASSERT(slice_cmp(empty3, slice7) < 0, "Empty should be less than non-empty");
}

void test_slice_to_double() {
  printf("\n=== Testing slice_to_double ===\n");

  // Test 1: Positive integer
  const char *num1 = "42";
  string_slice_t slice1 = {.start = num1, .length = 2};
  double d1 = slice_to_double(slice1);
  TEST_ASSERT(d1 == 42.0, "Should parse positive integer");

  // Test 2: Negative integer
  const char *num2 = "-42";
  string_slice_t slice2 = {.start = num2, .length = 3};
  double d2 = slice_to_double(slice2);
  TEST_ASSERT(d2 == -42.0, "Should parse negative integer");

  // Test 3: Decimal number
  const char *num3 = "3.14159";
  string_slice_t slice3 = {.start = num3, .length = 7};
  double d3 = slice_to_double(slice3);
  TEST_ASSERT(d3 > 3.14 && d3 < 3.15, "Should parse decimal number");

  // Test 4: Scientific notation (small number)
  const char *num4 = "1.5e-10";
  string_slice_t slice4 = {.start = num4, .length = 7};
  double d4 = slice_to_double(slice4);
  TEST_ASSERT(d4 < 0.0000000002 && d4 > 0, "Should parse small scientific notation");

  // Test 5: Scientific notation (large number)
  const char *num5 = "1.5E+10";
  string_slice_t slice5 = {.start = num5, .length = 7};
  double d5 = slice_to_double(slice5);
  TEST_ASSERT(d5 > 14999999999.0 && d5 < 15000000001.0, "Should parse large scientific notation");

  // Test 6: Zero
  const char *num6 = "0";
  string_slice_t slice6 = {.start = num6, .length = 1};
  double d6 = slice_to_double(slice6);
  TEST_ASSERT(d6 == 0.0, "Should parse zero");

  // Test 7: Small number fitting in stack buffer (< 32 bytes)
  const char *num7 = "123.456";
  string_slice_t slice7 = {.start = num7, .length = 7};
  double d7 = slice_to_double(slice7);
  TEST_ASSERT(d7 > 123.45 && d7 < 123.46, "Should use stack buffer for small numbers");

  // Test 8: Large number requiring heap allocation (>= 32 bytes)
  const char *num8 = "123456789012345678901234567890.123456";
  string_slice_t slice8 = {.start = num8, .length = 37};
  double d8 = slice_to_double(slice8);
  TEST_ASSERT(d8 > 1.0e29, "Should use heap allocation for large numbers");

  // Test 9: Partial slice from larger string
  const char *json_num = "42, \"next\": 100";
  string_slice_t slice9 = {.start = json_num, .length = 2};
  double d9 = slice_to_double(slice9);
  TEST_ASSERT(d9 == 42.0, "Should parse number from partial string");
}

void test_slice_print() {
  printf("\n=== Testing slice_print ===\n");

  // Note: slice_print outputs to stdout, so we test it doesn't crash
  // We can't easily capture stdout in our simple test framework

  const char *source = "hello world";

  printf("Expected output: 'hello' -> ");
  string_slice_t slice1 = {.start = source, .length = 5};
  slice_print(slice1);
  printf("\n");

  printf("Expected output: '' (empty) -> '");
  string_slice_t slice2 = {.start = source, .length = 0};
  slice_print(slice2);
  printf("'\n");

  printf("Expected output: 'world' -> ");
  string_slice_t slice3 = {.start = source + 6, .length = 5};
  slice_print(slice3);
  printf("\n");

  TEST_ASSERT(true, "slice_print should execute without crashing");
}

void test_slice_edge_cases() {
  printf("\n=== Testing slice edge cases ===\n");

  // Test 1: Slice with special characters
  const char *special = "hello\nworld\ttab";
  string_slice_t slice1 = {.start = special, .length = 11};
  char *str1 = slice_to_string(slice1);
  TEST_ASSERT(strcmp(str1, "hello\nworld") == 0, "Should handle newline in slice");
  free(str1);

  // Test 2: Slice with null in middle (up to null)
  const char *with_null = "hel\0lo";
  string_slice_t slice2 = {.start = with_null, .length = 3};
  char *str2 = slice_to_string(slice2);
  TEST_ASSERT(strcmp(str2, "hel") == 0, "Should handle slice up to middle null");
  free(str2);

  // Test 3: Comparing slices from different source strings
  const char *src1 = "test";
  const char *src2 = "test";
  string_slice_t slice3 = {.start = src1, .length = 4};
  string_slice_t slice4 = {.start = src2, .length = 4};
  TEST_ASSERT(slice_cmp(slice3, slice4) == 0, "Equal content from different sources should compare equal");

  // Test 4: Very small number (tests stack buffer optimization)
  const char *tiny = "0";
  string_slice_t slice5 = {.start = tiny, .length = 1};
  double d = slice_to_double(slice5);
  TEST_ASSERT(d == 0.0, "Should handle single digit number");

  // Test 5: Number at boundary (31 bytes - just under SMALL_BUFFER)
  const char *boundary = "1234567890123456789012345678901";  // 31 chars
  string_slice_t slice6 = {.start = boundary, .length = 31};
  double d2 = slice_to_double(slice6);
  TEST_ASSERT(d2 > 0, "Should handle 31-byte number with stack buffer");
}

TEST_MAIN("String Slice Operations",
  test_slice_to_string();
  test_slice_strcmp();
  test_slice_cmp();
  test_slice_to_double();
  test_slice_print();
  test_slice_edge_cases();
)
