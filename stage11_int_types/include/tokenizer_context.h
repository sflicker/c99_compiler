#ifndef __TOKENIZER_CONTEXT
#define __TOKENIZER_CONTEXT

#include "token.h"

typedef struct {
    const char * text;
    int text_len;
    int pos;
    char curr_char;
    char next_char;
    int line;
    int col;
} TokenizerContext;

char advance(TokenizerContext * context);
TokenizerContext * init_tokenizer_context(const char * text);
// TokenList * get_tokens(TokenizerContext * context);
// Token * get_next_token(TokenizerContext * context);
void free_tokenizer_context(TokenizerContext * ctx);

#endif