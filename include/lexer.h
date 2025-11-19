#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SMALL_BUFFER 32

typedef enum {
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_LBRACKET,
  TOKEN_RBRACKET,
  TOKEN_COLON,
  TOKEN_COMMA,
  TOKEN_STRING,
  TOKEN_NUMBER,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_NULL,
  TOKEN_EOF,
  TOKEN_ERROR,
} token_type_t;

typedef struct {
  const char *start;
  size_t length;
} string_slice_t;

typedef struct {
  token_type_t type;
  int line, column;
  string_slice_t lexeme;
} token_t;

typedef struct {
  const char *start;
  const char *current;
  int line;
  int column;

  token_t last_token;
  bool has_peeked;
} lexer_t;

// util functions
static inline int is_digit(char ch) {
  return ch >= '0' && ch <= '9';
};
static inline int is_space(char ch) {
  return ch == ' ' || ch == '\n' || ch == '\t';
};

void skip_whitespace(lexer_t *);

// String slice helper functions
char *slice_to_string(string_slice_t);
int slice_strcmp(string_slice_t, char *);
int slice_cmp(string_slice_t, string_slice_t);
double slice_to_double(string_slice_t);
void slice_print(string_slice_t);


lexer_t lexer_init(const char *);
void lexer_free(lexer_t *);

void token_free(token_t *);

/* token_t tokenize_string(lexer_t *); */
/* token_t tokenize_number(lexer_t *); */
token_t next_token(lexer_t *);
token_t peek_token(lexer_t *);
void print_token(token_t *);
