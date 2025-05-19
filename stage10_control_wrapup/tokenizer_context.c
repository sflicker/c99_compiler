#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "token.h"
#include "tokenizer_context.h"

TokenizerContext * init_tokenizer_context(const char * text) {
    TokenizerContext * context = malloc(sizeof(TokenizerContext));
    context->text = text;
    context->text_len = strlen(text);
    context->pos = 0;
    context->line = 1;
    context->col = 1;
    context->curr_char = context->text[context->pos];
    context->next_char = context->text[(context->pos)+1];
    return context;
}

char advance(TokenizerContext * context) {
    context->pos++;
    context->curr_char = context->text[context->pos];
    context->next_char = context->text[(context->pos)+1];
    if (context->curr_char == '\n') {
        context->line++;
        context->col=1;
    }
    else {
        context->col++;
    }
    return context->curr_char;
}

// TokenList * get_tokens(TokenizerContext * context) {
//     TokenList * tokenList = malloc(sizeof(TokenList));

//     while(context->current) {
//         Token * token = get_next_token(context);
//         append_token(tokenList, token);
//     }

//     Token * eof = make_token(TOKEN_EOF, "");
//     append_token(tokenList, eof);
//     return tokenList;
// }

// Token * get_next_token(TokenizerContext * context) {
//     while(context->current) {
//         if (isspace(context->current)) {
//             advance(context);
//         }
//     }
// }


