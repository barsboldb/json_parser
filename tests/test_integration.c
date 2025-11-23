#include "test_framework.h"
#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TEST_SUITE_INIT()

// Helper function to read file contents
char* read_file(const char* filepath) {
  FILE* file = fopen(filepath, "r");
  if (!file) {
    printf("Failed to open file: %s\n", filepath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* content = malloc(length + 1);
  if (!content) {
    fclose(file);
    return NULL;
  }

  fread(content, 1, length, file);
  content[length] = '\0';
  fclose(file);

  return content;
}

void test_simple_json() {
  printf("\n=== Testing simple.json ===\n");

  char* json = read_file("samples/simple.json");
  TEST_ASSERT(json != NULL, "Should read simple.json file");

  lexer_t lexer = lexer_init(json);
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse(&parser);
  TEST_ASSERT(!parser.has_error, "Should parse without errors");
  TEST_ASSERT(value.type == JSON_OBJECT, "Root should be object");
  TEST_ASSERT(json_object_size(&value) == 3, "Object should have 3 entries");

  // Check "name" field
  json_value_t name = json_object_get(&value, "name");
  TEST_ASSERT(name.type == JSON_STRING, "name should be string");
  TEST_ASSERT(strcmp(name.string, "Barsbold") == 0, "name should be 'Barsbold'");

  // Check "age" field
  json_value_t age = json_object_get(&value, "age");
  TEST_ASSERT(age.type == JSON_NUMBER, "age should be number");
  TEST_ASSERT(age.number == 69.0, "age should be 69");

  // Check "profession" field
  json_value_t profession = json_object_get(&value, "profession");
  TEST_ASSERT(profession.type == JSON_NULL, "profession should be null");

  json_value_free(&value);
  lexer_free(&lexer);
  free(json);
}

void test_array_json() {
  printf("\n=== Testing array.json ===\n");

  char* json = read_file("samples/array.json");
  TEST_ASSERT(json != NULL, "Should read array.json file");

  lexer_t lexer = lexer_init(json);
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse(&parser);
  TEST_ASSERT(!parser.has_error, "Should parse without errors");
  TEST_ASSERT(value.type == JSON_ARRAY, "Root should be array");
  TEST_ASSERT(value.array.len == 4, "Array should have 4 elements");

  // Check array elements
  TEST_ASSERT(value.array.items[0].type == JSON_STRING, "First element should be string");
  TEST_ASSERT(strcmp(value.array.items[0].string, "Barsbold") == 0, "First element should be 'Barsbold'");

  TEST_ASSERT(value.array.items[1].type == JSON_NUMBER, "Second element should be number");
  TEST_ASSERT(value.array.items[1].number == 21.0, "Second element should be 21");

  TEST_ASSERT(value.array.items[2].type == JSON_BOOL, "Third element should be boolean");
  TEST_ASSERT(value.array.items[2].boolean == true, "Third element should be true");

  TEST_ASSERT(value.array.items[3].type == JSON_NULL, "Fourth element should be null");

  json_value_free(&value);
  lexer_free(&lexer);
  free(json);
}

void test_nested_json() {
  printf("\n=== Testing nested.json ===\n");

  char* json = read_file("samples/nested.json");
  TEST_ASSERT(json != NULL, "Should read nested.json file");

  lexer_t lexer = lexer_init(json);
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse(&parser);
  TEST_ASSERT(!parser.has_error, "Should parse without errors");
  TEST_ASSERT(value.type == JSON_OBJECT, "Root should be object");

  // Check company name
  json_value_t company = json_object_get(&value, "company");
  TEST_ASSERT(company.type == JSON_STRING, "company should be string");
  TEST_ASSERT(strcmp(company.string, "Tech Corp") == 0, "company should be 'Tech Corp'");

  // Check employees array
  json_value_t employees = json_object_get(&value, "employees");
  TEST_ASSERT(employees.type == JSON_ARRAY, "employees should be array");
  TEST_ASSERT(employees.array.len == 2, "employees array should have 2 elements");

  // Check first employee
  json_value_t first_employee = employees.array.items[0];
  TEST_ASSERT(first_employee.type == JSON_OBJECT, "First employee should be object");

  json_value_t emp_name = json_object_get(&first_employee, "name");
  TEST_ASSERT(emp_name.type == JSON_STRING, "Employee name should be string");
  TEST_ASSERT(strcmp(emp_name.string, "Alice Johnson") == 0, "Employee name should be 'Alice Johnson'");

  json_value_t emp_age = json_object_get(&first_employee, "age");
  TEST_ASSERT(emp_age.type == JSON_NUMBER, "Employee age should be number");
  TEST_ASSERT(emp_age.number == 28.0, "Employee age should be 28");

  json_value_t emp_active = json_object_get(&first_employee, "active");
  TEST_ASSERT(emp_active.type == JSON_BOOL, "Employee active should be boolean");
  TEST_ASSERT(emp_active.boolean == true, "Employee should be active");

  // Check skills array
  json_value_t skills = json_object_get(&first_employee, "skills");
  TEST_ASSERT(skills.type == JSON_ARRAY, "skills should be array");
  TEST_ASSERT(skills.array.len == 3, "skills array should have 3 elements");
  TEST_ASSERT(skills.array.items[0].type == JSON_STRING, "First skill should be string");
  TEST_ASSERT(strcmp(skills.array.items[0].string, "JavaScript") == 0, "First skill should be 'JavaScript'");

  // Check nested address object
  json_value_t address = json_object_get(&first_employee, "address");
  TEST_ASSERT(address.type == JSON_OBJECT, "address should be object");

  json_value_t city = json_object_get(&address, "city");
  TEST_ASSERT(city.type == JSON_STRING, "city should be string");
  TEST_ASSERT(strcmp(city.string, "San Francisco") == 0, "city should be 'San Francisco'");

  // Check departments object
  json_value_t departments = json_object_get(&value, "departments");
  TEST_ASSERT(departments.type == JSON_OBJECT, "departments should be object");

  json_value_t engineering = json_object_get(&departments, "engineering");
  TEST_ASSERT(engineering.type == JSON_OBJECT, "engineering should be object");

  json_value_t eng_head = json_object_get(&engineering, "head");
  TEST_ASSERT(eng_head.type == JSON_STRING, "engineering head should be string");
  TEST_ASSERT(strcmp(eng_head.string, "Alice Johnson") == 0, "engineering head should be 'Alice Johnson'");

  json_value_t eng_size = json_object_get(&engineering, "size");
  TEST_ASSERT(eng_size.type == JSON_NUMBER, "engineering size should be number");
  TEST_ASSERT(eng_size.number == 50.0, "engineering size should be 50");

  // Check other root fields
  json_value_t founded = json_object_get(&value, "founded");
  TEST_ASSERT(founded.type == JSON_NUMBER, "founded should be number");
  TEST_ASSERT(founded.number == 2010.0, "founded should be 2010");

  json_value_t is_public = json_object_get(&value, "public");
  TEST_ASSERT(is_public.type == JSON_BOOL, "public should be boolean");
  TEST_ASSERT(is_public.boolean == true, "public should be true");

  json_value_t revenue = json_object_get(&value, "revenue");
  TEST_ASSERT(revenue.type == JSON_NULL, "revenue should be null");

  json_value_free(&value);
  lexer_free(&lexer);
  free(json);
}

void test_complex_json() {
  printf("\n=== Testing complex.json ===\n");

  char* json = read_file("samples/complex.json");
  TEST_ASSERT(json != NULL, "Should read complex.json file");

  lexer_t lexer = lexer_init(json);
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse(&parser);
  TEST_ASSERT(!parser.has_error, "Should parse without errors");
  TEST_ASSERT(value.type == JSON_OBJECT, "Root should be object");

  // Check api_version
  json_value_t api_version = json_object_get(&value, "api_version");
  TEST_ASSERT(api_version.type == JSON_STRING, "api_version should be string");
  TEST_ASSERT(strcmp(api_version.string, "v2.1") == 0, "api_version should be 'v2.1'");

  // Check data object
  json_value_t data = json_object_get(&value, "data");
  TEST_ASSERT(data.type == JSON_OBJECT, "data should be object");

  // Check users array
  json_value_t users = json_object_get(&data, "users");
  TEST_ASSERT(users.type == JSON_ARRAY, "users should be array");
  TEST_ASSERT(users.array.len == 2, "users array should have 2 elements");

  // Check first user
  json_value_t user1 = users.array.items[0];
  TEST_ASSERT(user1.type == JSON_OBJECT, "First user should be object");

  json_value_t username = json_object_get(&user1, "username");
  TEST_ASSERT(username.type == JSON_STRING, "username should be string");
  TEST_ASSERT(strcmp(username.string, "user1") == 0, "username should be 'user1'");

  json_value_t profile = json_object_get(&user1, "profile");
  TEST_ASSERT(profile.type == JSON_OBJECT, "profile should be object");

  json_value_t bio = json_object_get(&profile, "bio");
  TEST_ASSERT(bio.type == JSON_NULL, "bio should be null");

  json_value_t verified = json_object_get(&profile, "verified");
  TEST_ASSERT(verified.type == JSON_BOOL, "verified should be boolean");
  TEST_ASSERT(verified.boolean == true, "verified should be true");

  json_value_t followers = json_object_get(&profile, "followers");
  TEST_ASSERT(followers.type == JSON_NUMBER, "followers should be number");
  TEST_ASSERT(followers.number == 1250.0, "followers should be 1250");

  json_value_t tags = json_object_get(&profile, "tags");
  TEST_ASSERT(tags.type == JSON_ARRAY, "tags should be array");
  TEST_ASSERT(tags.array.len == 2, "tags array should have 2 elements");

  // Check posts array
  json_value_t posts = json_object_get(&data, "posts");
  TEST_ASSERT(posts.type == JSON_ARRAY, "posts should be array");
  TEST_ASSERT(posts.array.len == 1, "posts array should have 1 element");

  json_value_t post = posts.array.items[0];
  TEST_ASSERT(post.type == JSON_OBJECT, "post should be object");

  json_value_t comments = json_object_get(&post, "comments");
  TEST_ASSERT(comments.type == JSON_ARRAY, "comments should be array");
  TEST_ASSERT(comments.array.len == 2, "comments array should have 2 elements");

  json_value_t first_comment = comments.array.items[0];
  TEST_ASSERT(first_comment.type == JSON_OBJECT, "First comment should be object");

  json_value_t comment_text = json_object_get(&first_comment, "text");
  TEST_ASSERT(comment_text.type == JSON_STRING, "comment text should be string");
  TEST_ASSERT(strcmp(comment_text.string, "Great post!") == 0, "comment text should be 'Great post!'");

  // Check metadata
  json_value_t metadata = json_object_get(&value, "metadata");
  TEST_ASSERT(metadata.type == JSON_OBJECT, "metadata should be object");

  json_value_t cached = json_object_get(&metadata, "cached");
  TEST_ASSERT(cached.type == JSON_BOOL, "cached should be boolean");
  TEST_ASSERT(cached.boolean == false, "cached should be false");

  json_value_free(&value);
  lexer_free(&lexer);
  free(json);
}

void test_edge_cases_json() {
  printf("\n=== Testing edge_cases.json ===\n");

  char* json = read_file("samples/edge_cases.json");
  TEST_ASSERT(json != NULL, "Should read edge_cases.json file");

  lexer_t lexer = lexer_init(json);
  parser_t parser = parser_init(&lexer);
  parser.current_token = next_token(&lexer);

  json_value_t value = parse(&parser);
  TEST_ASSERT(!parser.has_error, "Should parse without errors");
  TEST_ASSERT(value.type == JSON_OBJECT, "Root should be object");

  // Check empty values
  json_value_t empty_string = json_object_get(&value, "empty_string");
  TEST_ASSERT(empty_string.type == JSON_STRING, "empty_string should be string");
  TEST_ASSERT(strcmp(empty_string.string, "") == 0, "empty_string should be empty");

  json_value_t empty_array = json_object_get(&value, "empty_array");
  TEST_ASSERT(empty_array.type == JSON_ARRAY, "empty_array should be array");
  TEST_ASSERT(empty_array.array.len == 0, "empty_array should have 0 elements");

  json_value_t empty_object = json_object_get(&value, "empty_object");
  TEST_ASSERT(empty_object.type == JSON_OBJECT, "empty_object should be object");
  TEST_ASSERT(json_object_size(&empty_object) == 0, "empty_object should have 0 entries");

  // Check nested empty
  json_value_t nested_empty = json_object_get(&value, "nested_empty");
  TEST_ASSERT(nested_empty.type == JSON_OBJECT, "nested_empty should be object");

  json_value_t nested_arr = json_object_get(&nested_empty, "arr");
  TEST_ASSERT(nested_arr.type == JSON_ARRAY, "nested arr should be array");
  TEST_ASSERT(nested_arr.array.len == 0, "nested arr should be empty");

  json_value_t nested_obj = json_object_get(&nested_empty, "obj");
  TEST_ASSERT(nested_obj.type == JSON_OBJECT, "nested obj should be object");
  TEST_ASSERT(json_object_size(&nested_obj) == 0, "nested obj should be empty");

  // Check numbers
  json_value_t numbers = json_object_get(&value, "numbers");
  TEST_ASSERT(numbers.type == JSON_OBJECT, "numbers should be object");

  json_value_t integer = json_object_get(&numbers, "integer");
  TEST_ASSERT(integer.type == JSON_NUMBER, "integer should be number");
  TEST_ASSERT(integer.number == 42.0, "integer should be 42");

  json_value_t negative = json_object_get(&numbers, "negative");
  TEST_ASSERT(negative.type == JSON_NUMBER, "negative should be number");
  TEST_ASSERT(negative.number == -123.0, "negative should be -123");

  json_value_t decimal = json_object_get(&numbers, "decimal");
  TEST_ASSERT(decimal.type == JSON_NUMBER, "decimal should be number");
  TEST_ASSERT(decimal.number > 3.14 && decimal.number < 3.15, "decimal should be approximately 3.14159");

  json_value_t scientific = json_object_get(&numbers, "scientific");
  TEST_ASSERT(scientific.type == JSON_NUMBER, "scientific should be number");
  TEST_ASSERT(scientific.number > 0.0001 && scientific.number < 0.0002, "scientific should be approximately 0.000123");

  json_value_t zero = json_object_get(&numbers, "zero");
  TEST_ASSERT(zero.type == JSON_NUMBER, "zero should be number");
  TEST_ASSERT(zero.number == 0.0, "zero should be 0");

  // Check deeply nested
  json_value_t deeply_nested = json_object_get(&value, "deeply_nested");
  TEST_ASSERT(deeply_nested.type == JSON_OBJECT, "deeply_nested should be object");

  json_value_t level1 = json_object_get(&deeply_nested, "level1");
  TEST_ASSERT(level1.type == JSON_OBJECT, "level1 should be object");

  json_value_t level2 = json_object_get(&level1, "level2");
  TEST_ASSERT(level2.type == JSON_OBJECT, "level2 should be object");

  json_value_t level3 = json_object_get(&level2, "level3");
  TEST_ASSERT(level3.type == JSON_OBJECT, "level3 should be object");

  json_value_t level4 = json_object_get(&level3, "level4");
  TEST_ASSERT(level4.type == JSON_OBJECT, "level4 should be object");

  json_value_t deep_value = json_object_get(&level4, "value");
  TEST_ASSERT(deep_value.type == JSON_STRING, "deep value should be string");
  TEST_ASSERT(strcmp(deep_value.string, "deep") == 0, "deep value should be 'deep'");

  // Check mixed array
  json_value_t mixed_array = json_object_get(&value, "mixed_array");
  TEST_ASSERT(mixed_array.type == JSON_ARRAY, "mixed_array should be array");
  TEST_ASSERT(mixed_array.array.len == 6, "mixed_array should have 6 elements");

  TEST_ASSERT(mixed_array.array.items[0].type == JSON_NUMBER, "Element 0 should be number");
  TEST_ASSERT(mixed_array.array.items[1].type == JSON_STRING, "Element 1 should be string");
  TEST_ASSERT(mixed_array.array.items[2].type == JSON_BOOL, "Element 2 should be boolean");
  TEST_ASSERT(mixed_array.array.items[3].type == JSON_NULL, "Element 3 should be null");
  TEST_ASSERT(mixed_array.array.items[4].type == JSON_OBJECT, "Element 4 should be object");
  TEST_ASSERT(mixed_array.array.items[5].type == JSON_ARRAY, "Element 5 should be array");

  json_value_t nested_array = mixed_array.array.items[5];
  TEST_ASSERT(nested_array.array.len == 3, "nested array should have 3 elements");

  json_value_free(&value);
  lexer_free(&lexer);
  free(json);
}

TEST_MAIN("Integration",
  test_simple_json();
  test_array_json();
  test_nested_json();
  test_complex_json();
  test_edge_cases_json();
)
