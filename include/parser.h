#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "json.h"
#include "mem_pool.h"

typedef struct {
  lexer_t *lexer;
  token_t current_token;
  bool has_error;
  char error_message[256];
  mem_pool_t *pool;
  bool owns_pool;  // whether the parser owns the pool
} parser_t;

parser_t parser_init(lexer_t *);
void parser_free(parser_t *);

json_value_t parse(parser_t *);

json_value_t parse_value(parser_t *);
json_value_t parse_object(parser_t *);
json_value_t parse_array(parser_t *);
json_value_t parse_string(parser_t *);
json_value_t parse_number(parser_t *);
json_value_t parse_boolean(parser_t *);
json_value_t parse_null(parser_t *);

static inline void advance(parser_t *parser) {
  if (!parser->has_error && parser->current_token.type != TOKEN_EOF) {
    token_free(&parser->current_token);
    parser->current_token = next_token(parser->lexer);
  }
}
static inline bool check(parser_t *parser, token_type_t type) {
  return parser->current_token.type == type;
}
static inline bool match(parser_t *parser, token_type_t type) {
  if (check(parser, type)) {
    advance(parser);
    return true;
  }
  return false;
}
void parser_error(parser_t *, const char *);

#endif
