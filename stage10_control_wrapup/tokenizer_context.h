#ifndef __TOKENIZER_CONTEXT
#define __TOKENIZER_CONTEXT

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

#endif