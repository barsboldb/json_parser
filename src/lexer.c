#include "../include/lexer.h"

lexer_t lexer_init(const char *input) {
  size_t len = strlen(input);
  char *in_str = malloc(len + 1);
  strcpy(in_str, input);
  lexer_t lexer = {
    .start = in_str,
    .current = in_str,
    .line = 1,
    .column = 1,
    .has_peeked = false,
    .last_token = {
      .lexeme = NULL,
    },
  };
  return lexer;
}

void lexer_free(lexer_t *lexer) {
  if (lexer->start) {
    free((char *)lexer->start);
  }
}

// Helper function to check if character is a digit
int is_space(char ch) { return ch == ' ' || ch == '\n' || ch == '\t'; }
int is_digit(char ch) { return ch >= '0' && ch <= '9'; }

// Helper function to skip whitespace
void skip_whitespace(lexer_t *lexer) {
  while (*lexer->current != '\0' && is_space(*lexer->current)) {
    if (*lexer->current == '\n') {
      lexer->line++;
      lexer->column = 1;
    } else {
      lexer->column++;
    }
    lexer->current++;
  }
}

// Improved number tokenization
token_t tokenize_number(lexer_t *lexer) {
  token_t token;
  const char *start = lexer->current;

  // Handle optional minus sign
  if (*lexer->current == '-') {
    lexer->current++;
    lexer->column++;
  }

  // Must have at least one digit
  if (!is_digit(*lexer->current)) {
    token.type = TOKEN_ERROR;
    token.line = lexer->line;
    token.column = lexer->column;
    token.lexeme = malloc(2);
    token.lexeme[0] = *lexer->current;
    token.lexeme[1] = '\0';
    return token;
  }

  // Handle integer part
  if (*lexer->current == '0') {
    // If starts with 0, it must be just 0 or 0.something
    lexer->current++;
    lexer->column++;
  } else {
    // Non-zero digit followed by more digits
    while (is_digit(*lexer->current)) {
      lexer->current++;
      lexer->column++;
    }
  }

  // Handle optional decimal part
  if (*lexer->current == '.') {
    lexer->current++;
    lexer->column++;

    // Must have at least one digit after decimal point
    if (!is_digit(*lexer->current)) {
      token.type = TOKEN_ERROR;
      token.line = lexer->line;
      token.column = lexer->column;
      token.lexeme = malloc(2);
      token.lexeme[0] = *lexer->current;
      token.lexeme[1] = '\0';
      return token;
    }

    while (is_digit(*lexer->current)) {
      lexer->current++;
      lexer->column++;
    }
  }

  // Handle optional exponent part
  if (*lexer->current == 'e' || *lexer->current == 'E') {
    lexer->current++;
    lexer->column++;

    // Optional + or - after e/E
    if (*lexer->current == '+' || *lexer->current == '-') {
      lexer->current++;
      lexer->column++;
    }

    // Must have at least one digit in exponent
    if (!is_digit(*lexer->current)) {
      token.type = TOKEN_ERROR;
      token.line = lexer->line;
      token.column = lexer->column;
      token.lexeme = malloc(2);
      token.lexeme[0] = *lexer->current;
      token.lexeme[1] = '\0';
      return token;
    }

    while (is_digit(*lexer->current)) {
      lexer->current++;
      lexer->column++;
    }
  }

  // Create token
  size_t len = lexer->current - start;
  token.type = TOKEN_NUMBER;
  token.line = lexer->line;
  token.column = lexer->column - len;
  token.lexeme = malloc(len + 1);
  memcpy(token.lexeme, start, len);
  token.lexeme[len] = '\0';

  return token;
}

// Improved string tokenization with escape sequence handling
token_t tokenize_string(lexer_t *lexer) {
  token_t token;
  const char *start;

  // Skip opening quote
  lexer->current++;
  lexer->column++;
  start = lexer->current;

  while (*lexer->current != '\0' && *lexer->current != '"') {
    if (*lexer->current == '\\') {
      // Skip escape sequence
      lexer->current++;
      lexer->column++;
      if (*lexer->current != '\0') {
        lexer->current++;
        lexer->column++;
      }
    } else {
      if (*lexer->current == '\n') {
        lexer->line++;
        lexer->column = 1;
      } else {
        lexer->column++;
      }
      lexer->current++;
    }
  }

  if (*lexer->current == '\0') {
    // Unterminated string
    token.type = TOKEN_ERROR;
    token.line = lexer->line;
    token.column = lexer->column;
    token.lexeme = malloc(20);
    strcpy(token.lexeme, "Unterminated string");
    return token;
  }

  // Calculate length and create lexeme
  size_t len = lexer->current - start;
  token.type = TOKEN_STRING;
  token.line = lexer->line;
  token.column = lexer->column - len - 1; // Adjust for opening quote
  token.lexeme = malloc(len + 1);
  memcpy(token.lexeme, start, len);
  token.lexeme[len] = '\0';

  // Skip closing quote
  lexer->current++;
  lexer->column++;

  return token;
}

void token_free(token_t *token) {
  if (token->lexeme) {
    free(token->lexeme);
    token->lexeme = NULL;
  }
}

token_t tokenize(lexer_t *lexer) {
  skip_whitespace(lexer);

  token_t token;
  token.line = lexer->line;
  token.column = lexer->column;

  if (*lexer->current == '\0') {
    token.type = TOKEN_EOF;
    token.lexeme = malloc(1);
    token.lexeme[0] = '\0';
    return token;
  }

  char ch = *lexer->current;

  // Check for numbers (including negative numbers)
  if (is_digit(ch) || (ch == '-' && is_digit(lexer->current[1]))) {
    return tokenize_number(lexer);
  }

  // Single character tokens
  switch (ch) {
  case '{':
    token.type = TOKEN_LBRACE;
    break;
  case '}':
    token.type = TOKEN_RBRACE;
    break;
  case '[':
    token.type = TOKEN_LBRACKET;
    break;
  case ']':
    token.type = TOKEN_RBRACKET;
    break;
  case ':':
    token.type = TOKEN_COLON;
    break;
  case ',':
    token.type = TOKEN_COMMA;
    break;
  case '"':
    return tokenize_string(lexer);
  case 't':
    // Check for "true"
    if (strncmp(lexer->current, "true", 4) == 0) {
      token.type = TOKEN_TRUE;
      token.lexeme = malloc(5);
      strcpy(token.lexeme, "true");
      lexer->current += 4;
      lexer->column += 4;
      return token;
    }
    token.type = TOKEN_ERROR;
    break;
  case 'f':
    // Check for "false"
    if (strncmp(lexer->current, "false", 5) == 0) {
      token.type = TOKEN_FALSE;
      token.lexeme = malloc(6);
      strcpy(token.lexeme, "false");
      lexer->current += 5;
      lexer->column += 5;
      return token;
    }
    token.type = TOKEN_ERROR;
    break;
  case 'n':
    // Check for "null"
    if (strncmp(lexer->current, "null", 4) == 0) {
      token.type = TOKEN_NULL;
      token.lexeme = malloc(5);
      strcpy(token.lexeme, "null");
      lexer->current += 4;
      lexer->column += 4;
      return token;
    }
    token.type = TOKEN_ERROR;
    break;
  default:
    token.type = TOKEN_ERROR;
  }

  // For single character tokens and errors
  if (token.type != TOKEN_STRING) {
    token.lexeme = malloc(2);
    token.lexeme[0] = ch;
    token.lexeme[1] = '\0';
    lexer->current++;
    lexer->column++;
  }

  return token;
}

// 
token_t next_token(lexer_t *lexer) {
  if (lexer->has_peeked) {
    lexer->has_peeked = false;
    return lexer->last_token;
  }

  token_free(&lexer->last_token);

  token_t token = tokenize(lexer);
  lexer->last_token = token;
  return token;
}

token_t peek_token(lexer_t *lexer) {
  if (!lexer->has_peeked) {
    lexer->last_token = next_token(lexer);
    lexer->has_peeked = true;
  }
  return lexer->last_token;
}

void print_token(token_t *token) {
  switch (token->type) {
  case TOKEN_LBRACE:
    printf("TOKEN_LBRACE col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_RBRACE:
    printf("TOKEN_RBRACE col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_LBRACKET:
    printf("TOKEN_LBRACKET col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_RBRACKET:
    printf("TOKEN_RBRACKET col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_COLON:
    printf("TOKEN_COLON col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_COMMA:
    printf("TOKEN_COMMA col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_STRING:
    printf("TOKEN_STRING col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_NUMBER:
    printf("TOKEN_NUMBER col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_TRUE:
    printf("TOKEN_TRUE col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_FALSE:
    printf("TOKEN_FALSE col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_NULL:
    printf("TOKEN_NULL col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  case TOKEN_EOF:
    printf("TOKEN_EOF col: %d lin: %d lexeme: %s\n", token->column, token->line,
           token->lexeme);
    break;
  case TOKEN_ERROR:
    printf("TOKEN_ERROR col: %d lin: %d lexeme: %s\n", token->column,
           token->line, token->lexeme);
    break;
  }
}
