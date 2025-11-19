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

__attribute__((cold))
void parser_error(parser_t *parser, const char *msg) {
  parser->has_error = true;
  snprintf(parser->error_message, sizeof(parser->error_message),
           "Parse error at line %d, column %d: %s",
           parser->current_token.line, parser->current_token.column, msg);
}

json_value_t parse_string(parser_t *parser) {
  if (!check(parser, TOKEN_STRING)) {
    parser_error(parser, "Expected string");
  }

  char *str = slice_to_string(parser->current_token.lexeme);
  json_value_t value = json_value_string(str);
  advance(parser);
  return value;
}

json_value_t parse_number(parser_t *parser) {
  if (!check(parser, TOKEN_NUMBER)) {
    parser_error(parser, "Expected number");
  }

  double num = slice_to_double(parser->current_token.lexeme);
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

json_value_t parse_value(parser_t *parser) {
  switch (parser->current_token.type) {
    case TOKEN_STRING:
      return parse_string(parser);
    case TOKEN_NUMBER:
      return parse_number(parser);
    case TOKEN_TRUE:
    case TOKEN_FALSE:
      return parse_boolean(parser);
    case TOKEN_NULL:
      return parse_null(parser);
    case TOKEN_LBRACKET:
      return parse_array(parser);
    case TOKEN_LBRACE:
      return parse_object(parser);
    default:
      parser_error(parser, "Unexpected token");
      return json_value_init(JSON_NULL);
  }
}

json_value_t parse_array(parser_t *parser) {
  if (!check(parser, TOKEN_LBRACKET)) {
    parser_error(parser, "Expected '['");
    return json_value_array(0);
  }
  advance(parser);

  json_value_t array = json_value_array(0);

  // Check for empty array
  if (check(parser, TOKEN_RBRACKET)) {
    advance(parser);
    return array;
  }

  while (true) {
    json_value_t element = parse_value(parser);
    if (parser->has_error) {
      return array;
    }

    json_array_push(&array, element);

    if (check(parser, TOKEN_COMMA)) {
      advance(parser);
      continue;
    } else if (check(parser, TOKEN_RBRACKET)) {
      break;
    } else {
      parser_error(parser, "Expected ',' or ']' in array");
      return array;
    }
  }

  if (!check(parser, TOKEN_RBRACKET)) {
    parser_error(parser, "Expected ']'");
  }
  advance(parser);

  return array;
}

json_value_t parse_object(parser_t *parser) {
  if (!check(parser, TOKEN_LBRACE)) {
    parser_error(parser, "Expected '{'");
    return json_value_object(0);
  }
  advance(parser);

  json_value_t object = json_value_object(0);

  if (check(parser, TOKEN_RBRACE)) {
    advance(parser);
    return object;
  }

  while (true) {
    if (parser->has_error) {
      return object;
    }
    if (!check(parser, TOKEN_STRING)) {
      parser_error(parser, "Expected string key in object");
      return object;
    }

    char *key = slice_to_string(parser->current_token.lexeme);
    advance(parser);

    if (!check(parser, TOKEN_COLON)) {
      parser_error(parser, "Expected ':'");
      return object;
    }

    advance(parser);
    json_value_t value = parse_value(parser);
    json_object_set(&object, key, value);

    if (check(parser, TOKEN_COMMA)) {
      advance(parser);
      continue;
    } else if (check(parser, TOKEN_RBRACE)) {
      break;
    } else {
      parser_error(parser, "Expected ',' or '}' in object");
      return object;
    }
  }

  advance(parser);  // Consume the closing '}'
  return object;
}

json_value_t parse(parser_t *parser) {
  return parse_value(parser);
}
