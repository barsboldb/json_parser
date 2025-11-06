#include "test_framework.h"
#include "../include/parser.h"
#include <string.h>
#include <math.h>

TEST_SUITE_INIT()

void test_parse_string() {
  printf("\n=== Testing parse_string ===\n");

  // Test 1: Simple string
  lexer_t lexer = lexer_init("\"hello\"");
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse_string(&parser);
  TEST_ASSERT(value.type == JSON_STRING, "Should parse string type");
  TEST_ASSERT(strcmp(value.string, "hello") == 0, "String value should be correct");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  json_value_free(&value);
  lexer_free(&lexer);
}

void test_parse_number() {
  printf("\n=== Testing parse_number ===\n");

  // Test 1: Positive integer
  lexer_t lexer = lexer_init("42");
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse_number(&parser);
  TEST_ASSERT(value.type == JSON_NUMBER, "Should parse number type");
  TEST_ASSERT(value.number == 42.0, "Number value should be 42");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  lexer_free(&lexer);

  // Test 2: Negative integer
  lexer = lexer_init("-42");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_number(&parser);
  TEST_ASSERT(value.type == JSON_NUMBER, "Should parse negative number");
  TEST_ASSERT(value.number == -42.0, "Negative number should be correct");

  lexer_free(&lexer);

  // Test 3: Decimal number
  lexer = lexer_init("3.14");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_number(&parser);
  TEST_ASSERT(value.type == JSON_NUMBER, "Should parse decimal number");
  TEST_ASSERT(fabs(value.number - 3.14) < 0.001, "Decimal value should be correct");

  lexer_free(&lexer);

  // Test 4: Scientific notation
  lexer = lexer_init("1.5e10");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_number(&parser);
  TEST_ASSERT(value.type == JSON_NUMBER, "Should parse scientific notation");
  TEST_ASSERT(value.number == 1.5e10, "Scientific notation should be correct");

  lexer_free(&lexer);

  // Test 5: Zero
  lexer = lexer_init("0");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_number(&parser);
  TEST_ASSERT(value.type == JSON_NUMBER, "Should parse zero");
  TEST_ASSERT(value.number == 0.0, "Zero should be correct");

  lexer_free(&lexer);

  // Test 6: Error - not a number token
  lexer = lexer_init("\"hello\"");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_number(&parser);
  TEST_ASSERT(parser.has_error, "Should have error when token is not number");

  lexer_free(&lexer);
}

void test_parse_boolean() {
  printf("\n=== Testing parse_boolean ===\n");

  // Test 1: Parse true
  lexer_t lexer = lexer_init("true");
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse_boolean(&parser);
  TEST_ASSERT(value.type == JSON_BOOL, "Should parse boolean type");
  TEST_ASSERT(value.boolean == true, "Boolean value should be true");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  lexer_free(&lexer);

  // Test 2: Parse false
  lexer = lexer_init("false");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_boolean(&parser);
  TEST_ASSERT(value.type == JSON_BOOL, "Should parse false boolean type");
  TEST_ASSERT(value.boolean == false, "Boolean value should be false");

  lexer_free(&lexer);

  // Test 3: Error - not a boolean token
  lexer = lexer_init("123");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_boolean(&parser);
  TEST_ASSERT(parser.has_error, "Should have error when token is not boolean");

  lexer_free(&lexer);
}

void test_parse_null() {
  printf("\n=== Testing parse_null ===\n");

  // Test 1: Parse null
  lexer_t lexer = lexer_init("null");
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse_null(&parser);
  TEST_ASSERT(value.type == JSON_NULL, "Should parse null type");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  lexer_free(&lexer);

  // Test 2: Error - not a null token
  lexer = lexer_init("123");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_null(&parser);
  TEST_ASSERT(parser.has_error, "Should have error when token is not null");

  lexer_free(&lexer);
}

void test_parser_helper_functions() {
  printf("\n=== Testing parser helper functions ===\n");

  // Test check() function
  lexer_t lexer = lexer_init("42");
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  TEST_ASSERT(check(&parser, TOKEN_NUMBER), "check() should return true for matching type");
  TEST_ASSERT(!check(&parser, TOKEN_STRING), "check() should return false for non-matching type");

  // Test match() function
  TEST_ASSERT(match(&parser, TOKEN_NUMBER), "match() should return true and advance for matching type");
  TEST_ASSERT(parser.current_token.type == TOKEN_EOF, "Token should advance to EOF after match");

  lexer_free(&lexer);

  // Test advance() function
  lexer = lexer_init("42 true");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  TEST_ASSERT(parser.current_token.type == TOKEN_NUMBER, "Initial token should be number");
  advance(&parser);
  TEST_ASSERT(parser.current_token.type == TOKEN_TRUE, "After advance, token should be true");

  lexer_free(&lexer);
}

void test_parser_error_handling() {
  printf("\n=== Testing parser error handling ===\n");

  // Test parser_error() function
  lexer_t lexer = lexer_init("123");
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  TEST_ASSERT(!parser.has_error, "Parser should not have error initially");

  parser_error(&parser, "Test error message");
  TEST_ASSERT(parser.has_error, "Parser should have error after parser_error()");
  TEST_ASSERT(strlen(parser.error_message) > 0, "Error message should be set");

  lexer_free(&lexer);
}

void test_parse_simple_values() {
  printf("\n=== Testing parse_value with simple values ===\n");

  // Test 1: Parse string value
  lexer_t lexer = lexer_init("\"hello\"");
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse_value(&parser);
  TEST_ASSERT(value.type == JSON_STRING, "parse_value should parse string");
  TEST_ASSERT(strcmp(value.string, "hello") == 0, "String value should be correct");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 2: Parse number value
  lexer = lexer_init("42");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_value(&parser);
  TEST_ASSERT(value.type == JSON_NUMBER, "parse_value should parse number");
  TEST_ASSERT(value.number == 42.0, "Number value should be correct");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  lexer_free(&lexer);

  // Test 3: Parse boolean true
  lexer = lexer_init("true");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_value(&parser);
  TEST_ASSERT(value.type == JSON_BOOL, "parse_value should parse boolean");
  TEST_ASSERT(value.boolean == true, "Boolean value should be true");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  lexer_free(&lexer);

  // Test 4: Parse boolean false
  lexer = lexer_init("false");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_value(&parser);
  TEST_ASSERT(value.type == JSON_BOOL, "parse_value should parse false");
  TEST_ASSERT(value.boolean == false, "Boolean value should be false");

  lexer_free(&lexer);

  // Test 5: Parse null value
  lexer = lexer_init("null");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_value(&parser);
  TEST_ASSERT(value.type == JSON_NULL, "parse_value should parse null");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  lexer_free(&lexer);

  // Test 6: Parse array value
  lexer = lexer_init("[1, 2, 3]");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_value(&parser);
  TEST_ASSERT(value.type == JSON_ARRAY, "parse_value should parse array");
  TEST_ASSERT(value.array.len == 3, "Array should have 3 elements");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 7: Parse nested array
  lexer = lexer_init("[[1], [2]]");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_value(&parser);
  TEST_ASSERT(value.type == JSON_ARRAY, "parse_value should parse nested array");
  TEST_ASSERT(value.array.len == 2, "Outer array should have 2 elements");
  TEST_ASSERT(value.array.items[0].type == JSON_ARRAY, "First element should be array");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 8: Error - unexpected token
  lexer = lexer_init("}");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_value(&parser);
  TEST_ASSERT(parser.has_error, "Should error on unexpected token");
  TEST_ASSERT(value.type == JSON_NULL, "Should return null on error");

  lexer_free(&lexer);
}

void test_parse_array() {
  printf("\n=== Testing parse_array ===\n");

  // Test 1: Empty array
  lexer_t lexer = lexer_init("[]");
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse_array(&parser);
  TEST_ASSERT(value.type == JSON_ARRAY, "Empty array should have JSON_ARRAY type");
  TEST_ASSERT(value.array.len == 0, "Empty array should have length 0");
  TEST_ASSERT(!parser.has_error, "Should not have error for empty array");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 2: Array with single number
  lexer = lexer_init("[42]");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_array(&parser);
  TEST_ASSERT(value.type == JSON_ARRAY, "Should parse array type");
  TEST_ASSERT(value.array.len == 1, "Array should have 1 element");
  TEST_ASSERT(value.array.items[0].type == JSON_NUMBER, "First element should be number");
  TEST_ASSERT(value.array.items[0].number == 42.0, "Number value should be 42");
  TEST_ASSERT(!parser.has_error, "Should not have error");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 3: Array with multiple numbers
  lexer = lexer_init("[1, 2, 3]");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_array(&parser);
  TEST_ASSERT(value.type == JSON_ARRAY, "Should parse array with multiple elements");
  TEST_ASSERT(value.array.len == 3, "Array should have 3 elements");
  TEST_ASSERT(value.array.items[0].number == 1.0, "First element should be 1");
  TEST_ASSERT(value.array.items[1].number == 2.0, "Second element should be 2");
  TEST_ASSERT(value.array.items[2].number == 3.0, "Third element should be 3");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 4: Array with mixed types
  lexer = lexer_init("[1, \"hello\", true, null]");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_array(&parser);
  TEST_ASSERT(value.type == JSON_ARRAY, "Should parse array with mixed types");
  TEST_ASSERT(value.array.len == 4, "Array should have 4 elements");
  TEST_ASSERT(value.array.items[0].type == JSON_NUMBER, "First element should be number");
  TEST_ASSERT(value.array.items[1].type == JSON_STRING, "Second element should be string");
  TEST_ASSERT(strcmp(value.array.items[1].string, "hello") == 0, "String value should be correct");
  TEST_ASSERT(value.array.items[2].type == JSON_BOOL, "Third element should be boolean");
  TEST_ASSERT(value.array.items[2].boolean == true, "Boolean value should be true");
  TEST_ASSERT(value.array.items[3].type == JSON_NULL, "Fourth element should be null");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 5: Nested arrays
  lexer = lexer_init("[[1, 2], [3, 4]]");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_array(&parser);
  TEST_ASSERT(value.type == JSON_ARRAY, "Should parse nested array");
  TEST_ASSERT(value.array.len == 2, "Outer array should have 2 elements");
  TEST_ASSERT(value.array.items[0].type == JSON_ARRAY, "First element should be array");
  TEST_ASSERT(value.array.items[0].array.len == 2, "First nested array should have 2 elements");
  TEST_ASSERT(value.array.items[0].array.items[0].number == 1.0, "First nested element should be 1");
  TEST_ASSERT(value.array.items[0].array.items[1].number == 2.0, "Second nested element should be 2");
  TEST_ASSERT(value.array.items[1].type == JSON_ARRAY, "Second element should be array");
  TEST_ASSERT(value.array.items[1].array.len == 2, "Second nested array should have 2 elements");
  TEST_ASSERT(value.array.items[1].array.items[0].number == 3.0, "Third nested element should be 3");
  TEST_ASSERT(value.array.items[1].array.items[1].number == 4.0, "Fourth nested element should be 4");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 6: Array with whitespace
  lexer = lexer_init("[ 1 , 2 , 3 ]");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_array(&parser);
  TEST_ASSERT(value.type == JSON_ARRAY, "Should handle whitespace in array");
  TEST_ASSERT(value.array.len == 3, "Array with whitespace should have 3 elements");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 7: Error - missing opening bracket
  lexer = lexer_init("1, 2, 3]");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_array(&parser);
  TEST_ASSERT(parser.has_error, "Should error on missing opening bracket");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 8: Error - missing closing bracket
  lexer = lexer_init("[1, 2, 3");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_array(&parser);
  TEST_ASSERT(parser.has_error, "Should error on missing closing bracket");

  json_value_free(&value);
  lexer_free(&lexer);

  // Test 9: Error - missing comma
  lexer = lexer_init("[1 2]");
  parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  value = parse_array(&parser);
  TEST_ASSERT(parser.has_error, "Should error on missing comma between elements");

  json_value_free(&value);
  lexer_free(&lexer);
}

void test_parse_object_placeholder() {
  printf("\n=== Testing parse_object (placeholder) ===\n");

  printf("  (parse_object tests require implementation)\n");

  // Test 1: Empty object
  // lexer_t lexer = lexer_init("{}");
  // parser_t parser = parser_init(&lexer);
  // parser.current_token = next_token(&lexer);
  // json_value_t value = parse_object(&parser);
  // TEST_ASSERT(value.type == JSON_OBJECT, "Should parse object type");
  // TEST_ASSERT(value.object.len == 0, "Empty object should have length 0");

  // Test 2: Object with single key-value pair
  // Test 3: Object with multiple key-value pairs
  // Test 4: Nested objects
}

TEST_MAIN("Parser",
  test_parse_string();
  test_parse_number();
  test_parse_boolean();
  test_parse_null();
  test_parser_helper_functions();
  test_parser_error_handling();
  test_parse_simple_values();
  test_parse_array();
  test_parse_object_placeholder();
)
