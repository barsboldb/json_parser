#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "json.h"

typedef struct {
  lexer_t *lexer;
  token_t current_token;
  bool has_error;
  char error_message[256];
} parser_t;

parser_t parser_init(lexer_t *);
void parser_free(parser_t *);

json_value_t *parse(parser_t *);

json_value_t *parse_value(parser_t *);
json_value_t *parse_object(parser_t *);
json_value_t *parse_array(parser_t *);
json_value_t *parse_string(parser_t *);
json_value_t *parse_number(parser_t *);
json_value_t *parse_boolean(parser_t *);
json_value_t *parse_null(parser_t *);

void advance(parser_t *);
bool match(parser_t *, token_type_t);
bool check(parser_t *, token_type_t);
void parser_error(parser_t *, const char *);
