#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tokenizer_context.h"

const char * program = 
    "int main() {\n"
    "    return 42;\n"
    "}";



int main() {
    printf("Starting test_tokenizer_context\n");
    TokenizerContext * ctx = init_tokenizer_context(program);
    assert(ctx->line == 1);
    assert(ctx->col == 1);
    assert(ctx->curr_char == 'i');
    assert(ctx->next_char == 'n');
    char c = advance(ctx);
    assert(c == 'n');
    assert(ctx->curr_char == 'n');
    assert(ctx->next_char == 't');
    assert(ctx->line == 1);
    assert(ctx->col == 2);
    for (int i=0; i<12;i++) {
        c = advance(ctx);
    }
    assert(ctx->line == 2);
    assert(ctx->col == 1);

    free_tokenizer_context(ctx);
    exit(0);
}
