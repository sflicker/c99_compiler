#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "token.h"
#include "tokenizer_context.h";

TokenizerContext * init_tokenizer_context(const char * text) {
    TokenizerContext * context = malloc(sizeof(TokenizerContext));
    context->text = text;
    context->pos = 0;
    context->line = 1;
    context->col = 1;
    context->current = context->text[context->pos];
    return context;
}

char advance(TokenizerContext * context) {
    context->pos++;
    context->current = context->text[context->pos];
    if (context->current == '\n') {
        context->line++;
        context->col=1;
    }
    else {
        context->col++;
    }
    return context->current;g
}

TokenList * get_tokens(TokenizerContext * context) {
    TokenList * tokenList = malloc(sizeof(TokenList));

    while(context->current) {
        Token * token = get_next_token(context);
        append_token(tokenList, token);
    }

    Token * eof = make_token(TOKEN_EOF, "");
    append_token(tokenList, eof);
    return tokenList;
}

Token * get_next_token(TokenizerContext * context) {
    while(context->current) {
        if (isspace(context->current)) {
            advance(context);
        }
    }
}


