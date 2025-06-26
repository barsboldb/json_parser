#include <stdio.h>
#include "../include/lexer.h"

int main(void) {
  const char* input = "{\"key\": 123,   \"name\": \"barsbold\", \"is_user\": false}";

  lexer_t lexer = lexer_init(input);

  token_t token;
  while ((token = next_token(&lexer)).type != TOKEN_EOF) {
    print_token(&token);
  }

  lexer_free(&lexer);
  return 0;
}
