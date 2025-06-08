#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "test_assert.h"
#include "tokenizer_context.h"

const char * current_test = NULL;

const char * program = 
    "int main() {\n"
    "    return 42;\n"
    "}";

void test_tokenizer_context() {
    TEST_MSG("Initializing context");
    TokenizerContext * ctx = init_tokenizer_context(program);
    TEST_ASSERT("Verifying ctx line is 1", ctx->line == 1);
    TEST_ASSERT("Verifying ctx col is 1", ctx->col == 1);
    TEST_ASSERT("Verifying ctx curr is 'i'", ctx->curr_char == 'i');
    TEST_ASSERT("Verifying ctx next is 'n'", ctx->next_char == 'n');

    TEST_MSG("Advancing");
    char c = advance(ctx);
    
    TEST_ASSERT("Verifying character return from advance is 'n'", c == 'n');
    TEST_ASSERT("Verifying ctx curr is 'n'", ctx->curr_char == 'n');
    TEST_ASSERT("Verifying ctx next is 't'", ctx->next_char == 't');
    TEST_ASSERT("Verifying ctx line is 1", ctx->line == 1);
    TEST_ASSERT("Verifying ctx col is 2", ctx->col == 2);

    TEST_MSG("Advancing multiple times to next line");
    for (int i=0; i<12;i++) {
        c = advance(ctx);
    }

    TEST_ASSERT("Verifying ctx line is 2", ctx->line == 2);
    TEST_ASSERT("Verifying ctx col is 1", ctx->col == 1);

    TEST_MSG("Freeing context");
    free_tokenizer_context(ctx);

}

int main() {
    RUN_TEST(test_tokenizer_context);
}
