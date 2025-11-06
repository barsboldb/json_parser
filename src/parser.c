#include "../include/parser.h"

parser_t parser_init(lexer_t *lexer) {
  parser_t parser = {
    .lexer = lexer,
  };
  return parser;
}
void parser_free(parser_t *parser) {
  token_free(&parser->current_token);
}

void parser_error(parser_t *parser, const char *msg) {
  parser->has_error = true;
  snprintf(parser->error_message, sizeof(parser->error_message),
           "Parse error at line %d, column %d: %s",
           parser->current_token.line, parser->current_token.column, msg);
}

void advance(parser_t *parser) {
  if (!parser->has_error && parser->current_token.type != TOKEN_EOF) {
    token_free(&parser->current_token);
    parser->current_token = next_token(parser->lexer);
  }
}

bool match(parser_t *parser, token_type_t type) {
  if (check(parser, type)) {
    advance(parser);
    return true;
  }
  return false;
}

bool check(parser_t *parser, token_type_t type) {
  return parser->current_token.type == type;
}

json_value_t parse_string(parser_t *parser) {
  if (!check(parser, TOKEN_STRING)) {
    parser_error(parser, "Expected string");
  }

  json_value_t value = json_value_string(parser->current_token.lexeme);
  advance(parser);
  return value;
}

json_value_t parse_number(parser_t *parser) {
  if (!check(parser, TOKEN_NUMBER)) {
    parser_error(parser, "Expected number");
  }

  double num = strtod(parser->current_token.lexeme, NULL);
  json_value_t value = json_value_number(num);
  advance(parser);
  return value;
}

json_value_t parse_boolean(parser_t *parser) {
  bool val;
  if (check(parser, TOKEN_TRUE)) {
    val = true;
  } else if (check(parser, TOKEN_FALSE)) {
    val = false;
  } else {
    parser_error(parser, "Expected boolean");
  }

  json_value_t value = json_value_bool(val);
  advance(parser);
  return value;
}

json_value_t parse_null(parser_t *parser) {
  if (!check(parser, TOKEN_NULL)) {
    parser_error(parser, "Expected null");
  }

  json_value_t value = json_value_init(JSON_NULL);
  advance(parser);
  return value;
}

json_value_t parse(parser_t *parser) {
  
}
