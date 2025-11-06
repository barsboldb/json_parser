#include "test_framework.h"
#include "../include/lexer.h"
#include <string.h>

TEST_SUITE_INIT()

void test_single_character_tokens() {
  printf("\n=== Testing single character tokens ===\n");
  
  lexer_t lexer = lexer_init("{");
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_LBRACE, "Left brace token");
  TEST_ASSERT(strcmp(token.lexeme, "{") == 0, "Left brace lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("}");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_RBRACE, "Right brace token");
  TEST_ASSERT(strcmp(token.lexeme, "}") == 0, "Right brace lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("[");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_LBRACKET, "Left bracket token");
  TEST_ASSERT(strcmp(token.lexeme, "[") == 0, "Left bracket lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("]");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_RBRACKET, "Right bracket token");
  TEST_ASSERT(strcmp(token.lexeme, "]") == 0, "Right bracket lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init(":");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_COLON, "Colon token");
  TEST_ASSERT(strcmp(token.lexeme, ":") == 0, "Colon lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init(",");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_COMMA, "Comma token");
  TEST_ASSERT(strcmp(token.lexeme, ",") == 0, "Comma lexeme");
  token_free(&token);
  lexer_free(&lexer);
}

void test_string_tokens() {
  printf("\n=== Testing string tokens ===\n");
  
  lexer_t lexer = lexer_init("\"hello\"");
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_STRING, "Simple string token");
  TEST_ASSERT(strcmp(token.lexeme, "hello") == 0, "Simple string lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("\"\"");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_STRING, "Empty string token");
  TEST_ASSERT(strcmp(token.lexeme, "") == 0, "Empty string lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("\"hello world\"");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_STRING, "String with spaces token");
  TEST_ASSERT(strcmp(token.lexeme, "hello world") == 0, "String with spaces lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("\"hello\\\"world\"");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_STRING, "String with escaped quote token");
  TEST_ASSERT(strcmp(token.lexeme, "hello\\\"world") == 0, "String with escaped quote lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("\"unterminated");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_ERROR, "Unterminated string error");
  token_free(&token);
  lexer_free(&lexer);
}

void test_number_tokens() {
  printf("\n=== Testing number tokens ===\n");
  
  lexer_t lexer = lexer_init("42");
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER, "Positive integer token");
  TEST_ASSERT(strcmp(token.lexeme, "42") == 0, "Positive integer lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("-42");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER, "Negative integer token");
  TEST_ASSERT(strcmp(token.lexeme, "-42") == 0, "Negative integer lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("0");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER, "Zero token");
  TEST_ASSERT(strcmp(token.lexeme, "0") == 0, "Zero lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("3.14");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER, "Decimal number token");
  TEST_ASSERT(strcmp(token.lexeme, "3.14") == 0, "Decimal number lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("-3.14");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER, "Negative decimal token");
  TEST_ASSERT(strcmp(token.lexeme, "-3.14") == 0, "Negative decimal lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("1e10");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER, "Scientific notation token");
  TEST_ASSERT(strcmp(token.lexeme, "1e10") == 0, "Scientific notation lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("1.5e-10");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER, "Complex scientific notation token");
  TEST_ASSERT(strcmp(token.lexeme, "1.5e-10") == 0, "Complex scientific notation lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("1.5E+10");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER, "Scientific notation with capital E token");
  TEST_ASSERT(strcmp(token.lexeme, "1.5E+10") == 0, "Scientific notation with capital E lexeme");
  token_free(&token);
  lexer_free(&lexer);
}

void test_boolean_tokens() {
  printf("\n=== Testing boolean tokens ===\n");
  
  lexer_t lexer = lexer_init("true");
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_TRUE, "True token");
  TEST_ASSERT(strcmp(token.lexeme, "true") == 0, "True lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("false");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_FALSE, "False token");
  TEST_ASSERT(strcmp(token.lexeme, "false") == 0, "False lexeme");
  token_free(&token);
  lexer_free(&lexer);
}

void test_null_token() {
  printf("\n=== Testing null token ===\n");
  
  lexer_t lexer = lexer_init("null");
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NULL, "Null token");
  TEST_ASSERT(strcmp(token.lexeme, "null") == 0, "Null lexeme");
  token_free(&token);
  lexer_free(&lexer);
}

void test_whitespace_handling() {
  printf("\n=== Testing whitespace handling ===\n");
  
  lexer_t lexer = lexer_init("  \t\n  42  ");
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER, "Number token after whitespace");
  TEST_ASSERT(strcmp(token.lexeme, "42") == 0, "Number lexeme after whitespace");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_EOF, "EOF after consuming whitespace");
  token_free(&token);
  lexer_free(&lexer);
}

void test_eof_token() {
  printf("\n=== Testing EOF token ===\n");
  
  lexer_t lexer = lexer_init("");
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_EOF, "EOF token on empty input");
  TEST_ASSERT(strcmp(token.lexeme, "") == 0, "EOF lexeme");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("42");
  token = next_token(&lexer);
  token_free(&token);
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_EOF, "EOF after consuming all tokens");
  token_free(&token);
  lexer_free(&lexer);
}

void test_error_tokens() {
  printf("\n=== Testing error tokens ===\n");
  
  lexer_t lexer = lexer_init("@");
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_ERROR, "Invalid character error");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("truthy");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_ERROR, "Invalid keyword error");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("falsish");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_ERROR, "Invalid false variant error");
  token_free(&token);
  lexer_free(&lexer);
  
  lexer = lexer_init("nullish");
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_ERROR, "Invalid null variant error");
  token_free(&token);
  lexer_free(&lexer);
}

void test_peek_functionality() {
  printf("\n=== Testing peek functionality ===\n");
  
  lexer_t lexer = lexer_init("42 true");
  
  token_t peeked = peek_token(&lexer);
  TEST_ASSERT(peeked.type == TOKEN_NUMBER, "Peek first token");
  TEST_ASSERT(strcmp(peeked.lexeme, "42") == 0, "Peek first token lexeme");
  
  peeked = peek_token(&lexer);
  TEST_ASSERT(peeked.type == TOKEN_NUMBER, "Peek same token again");
  TEST_ASSERT(strcmp(peeked.lexeme, "42") == 0, "Peek same token lexeme again");
  
  token_t consumed = next_token(&lexer);
  TEST_ASSERT(consumed.type == TOKEN_NUMBER, "Consume peeked token");
  TEST_ASSERT(strcmp(consumed.lexeme, "42") == 0, "Consume peeked token lexeme");
  token_free(&consumed);
  
  peeked = peek_token(&lexer);
  TEST_ASSERT(peeked.type == TOKEN_TRUE, "Peek next token");
  TEST_ASSERT(strcmp(peeked.lexeme, "true") == 0, "Peek next token lexeme");
  
  consumed = next_token(&lexer);
  TEST_ASSERT(consumed.type == TOKEN_TRUE, "Consume second token");
  TEST_ASSERT(strcmp(consumed.lexeme, "true") == 0, "Consume second token lexeme");
  token_free(&consumed);
  
  lexer_free(&lexer);
}

void test_complex_json() {
  printf("\n=== Testing complex JSON tokenization ===\n");
  
  lexer_t lexer = lexer_init("{\"name\": \"John\", \"age\": 30, \"active\": true}");
  
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_LBRACE, "Complex JSON - left brace");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_STRING && strcmp(token.lexeme, "name") == 0, "Complex JSON - name key");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_COLON, "Complex JSON - colon");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_STRING && strcmp(token.lexeme, "John") == 0, "Complex JSON - name value");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_COMMA, "Complex JSON - first comma");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_STRING && strcmp(token.lexeme, "age") == 0, "Complex JSON - age key");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_COLON, "Complex JSON - second colon");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_NUMBER && strcmp(token.lexeme, "30") == 0, "Complex JSON - age value");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_COMMA, "Complex JSON - second comma");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_STRING && strcmp(token.lexeme, "active") == 0, "Complex JSON - active key");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_COLON, "Complex JSON - third colon");
  token_free(&token);

  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_TRUE, "Complex JSON - active value");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_RBRACE, "Complex JSON - right brace");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.type == TOKEN_EOF, "Complex JSON - EOF");
  token_free(&token);
  
  lexer_free(&lexer);
}

void test_line_column_tracking() {
  printf("\n=== Testing line and column tracking ===\n");
  
  lexer_t lexer = lexer_init("{\n  \"key\": 42\n}");
  
  token_t token = next_token(&lexer);
  TEST_ASSERT(token.line == 1 && token.column == 1, "Line/column for opening brace");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.line == 2 && token.column == 3, "Line/column for string after newline");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.line == 2 && token.column == 8, "Line/column for colon");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.line == 2 && token.column == 10, "Line/column for number");
  token_free(&token);
  
  token = next_token(&lexer);
  TEST_ASSERT(token.line == 3 && token.column == 1, "Line/column for closing brace");
  token_free(&token);
  
  lexer_free(&lexer);
}

TEST_MAIN("Lexer", 
  test_single_character_tokens();
  test_string_tokens();
  test_number_tokens();
  test_boolean_tokens();
  test_null_token();
  test_whitespace_handling();
  test_eof_token();
  test_error_tokens();
  test_peek_functionality();
  test_complex_json();
  test_line_column_tracking();
)
