#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tokenizer.h"

const char * program_text = 
    "int main() {\n"
    "    return 42;\n"
    "}";

TokenType expected_tokens[] = { 
                    TOKEN_INT, 
                    TOKEN_IDENTIFIER, 
                    TOKEN_LPAREN, 
                    TOKEN_RPAREN,
                    TOKEN_LBRACE,
                    TOKEN_RETURN,
                    TOKEN_INT_LITERAL,
                    TOKEN_SEMICOLON,
                    TOKEN_RBRACE,
                    TOKEN_EOF
                };

int main() {
  tokenlist * tokens = tokenize(program_text);
  tokenlist_cursor * cursor=malloc(sizeof(tokenlist_cursor));
  tokenlist_cursor_init(cursor, tokens);
  int count = 0;
  while(tokenlist_cursor_has_next(cursor)) {
    printf("%d %s\n", count, cursor->current->value->text);
    assert(expected_tokens[count] == cursor->current->value->type);
    count++;
    tokenlist_cursor_next(cursor);
  }

//   assert(strcmp("int", cursor->current->value->text) == 0);
//   assert(TOKEN_INT == cursor->current->value->type);
  
//   tokenlist_cursor_next(cursor);
  
//   assert(strcmp("main", cursor->current->value->text) == 0);
//   assert(TOKEN_IDENTIFIER == cursor->current->value->type);

//   tokenlist_cursor_next(cursor);
  
//   assert(TOKEN_LPAREN == cursor->current->value->type);

//   tokenlist_cursor_next(cursor);
  
//   assert(TOKEN_RPAREN == cursor->current->value->type);

//   tokenlist_cursor_next(cursor);
  
//   assert(TOKEN_LBRACE == cursor->current->value->type);

//   tokenlist_cursor_next(cursor);
   
//   assert(TOKEN_RETURN == cursor->current->value->type);

//   tokenlist_cursor_next(cursor);
   
//   assert(TOKEN_INT_LITERAL == cursor->current->value->type);
//   assert(strcmp("42", cursor->current->value->text) == 0);

//   tokenlist_cursor_next(cursor);
   
//   assert(TOKEN_SEMICOLON == cursor->current->value->type);

//   tokenlist_cursor_next(cursor);
   
//   assert(TOKEN_RBRACE == cursor->current->value->type);

}