#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"

typedef struct {
    const char * text;
    int pos;
    char current;
    int line;
    int col;
} TokenizerContext;

TokenizerContext * init_tokenizer_context(const char * text);
TokenList * get_tokens(TokenizerContext * context);
Token * get_next_token(TokenizerContext * context);

void cleanup_token_list(TokenList * tokenList);
void tokenize(const char* code, TokenList * tokenList);
bool is_keyword(const char * word);

#endif