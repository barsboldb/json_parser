#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TOKEN_LBRACE,   //
  TOKEN_RBRACE,   //
  TOKEN_LBRACKET, //
  TOKEN_RBRACKET, //
  TOKEN_COLON,    //
  TOKEN_COMMA,    //
  TOKEN_STRING,   //
  TOKEN_NUMBER,   //
  TOKEN_TRUE,     //
  TOKEN_FALSE,    //
  TOKEN_NULL,     //
  TOKEN_EOF,
  TOKEN_ERROR,
} token_type;

typedef struct {
  token_type type;
  int line, column;
  char *lexeme;
} token_t;

typedef struct {
  const char *start;
  const char *current;
  int line;
  int column;
} lexer_t;

// util functions
int is_digit(char);
int is_space(char);
void skip_whitespace(lexer_t *);


lexer_t lexer_init(const char *);

void token_free(token_t *);

token_t tokenize_string(lexer_t *);
token_t tokenize_number(lexer_t *);
token_t next_token(lexer_t *);
void print_token(token_t *);
