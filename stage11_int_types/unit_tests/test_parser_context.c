#include <stdio.h>
#include <stdlib.h>

#include "list_util.h"
#include "token.h"
#include "tokenizer.h"
#include "parser_context.h"
#include "test_assert.h"


const char * program_text = 
    "int main() {\n"
    "    return 42;\n"
    "}";

void test_create_parser_context() {
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);
    TEST_ASSERT("Verifying ParserContext is not NULL", ctx != NULL);
    TEST_ASSERT("Verifying pos is zero", ctx->pos == 0);
    free_parser_context(ctx);
    tokenlist_free(tokens);
}

void test_peek() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    // test
    Token * token = peek(ctx);

    // verify
    TEST_ASSERT("Verifying token type", token->type == TOKEN_INT);
    TEST_ASSERT("Verifying token position", token->col == 1 && token->line == 1);
    
    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);
}

void test_is_current_token() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    // test
    TEST_ASSERT("Verifying correct result", is_current_token(ctx, TOKEN_INT) == true);
    
    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);
}

void test_is_next_token() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    // test
    TEST_ASSERT("Verifying correct result", is_next_token(ctx, TOKEN_IDENTIFIER) == true);
    
    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);
}

void test_advance_parser() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    advance_parser(ctx);
    Token * token = peek(ctx);

    // verify
    TEST_ASSERT("Verifying Correct Token Type", token->type == TOKEN_IDENTIFIER);

    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);

}

int main() {
    test_create_parser_context();
    test_peek();
    test_is_current_token();
    test_is_next_token();
    test_advance_parser();
}