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

void test_peek_next() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    // test
    Token * token = peek_next(ctx);

    // verify
    TEST_ASSERT("Verifying token type", token->type == TOKEN_IDENTIFIER);
    TEST_ASSERT("Verifying token text", strcmp("main", token->text)==0);
    
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

void test_match_token__false() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    // Test. this should not match. function should return 
    // false and parser should remain at the same position
    bool matched = match_token(ctx, TOKEN_EOF);

    TEST_ASSERT("Verifying Result", !matched);
    TEST_ASSERT("Verifying maintained position", ctx->pos == 0);

    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);

}

void test_match_token__true() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    // Test. this should match. function should return 
    // true and parser should move to the next position
    bool matched = match_token(ctx, TOKEN_INT);

    TEST_ASSERT("Verifying Result", matched);
    TEST_ASSERT("Verifying maintained position", ctx->pos == 1);

    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);

}

void test_is_current_token_a_type() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    // Test. this should match
    bool matched = is_current_token_a_ctype(ctx);

    TEST_ASSERT("Verifying Result", matched);

    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);

}

void test_expect_token() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    Token * token = expect_token(ctx, TOKEN_INT);

    // returned token should have the expected type
    TEST_ASSERT("Verifying Result", token->type == TOKEN_INT);

    // context should advance after finding the expected token
    TEST_ASSERT("Verifying correct current type", peek(ctx)->type == TOKEN_IDENTIFIER);

    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);

}

void test_expect_ctype_token() {
    // setup
    tokenlist * tokens = tokenize(program_text);
    ParserContext * ctx = create_parser_context(tokens);

    Token * token = expect_ctype_token(ctx);

        // returned token should have the expected type
    TEST_ASSERT("Verifying Result", token->type == TOKEN_INT);

    // context should advance after finding the expected token
    TEST_ASSERT("Verifying correct current type", peek(ctx)->type == TOKEN_IDENTIFIER);

    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);

}

int main() {
    test_create_parser_context();
    test_peek();
    test_peek_next();
    test_is_current_token();
    test_is_next_token();
    test_advance_parser();
    test_match_token__false();
    test_match_token__true();
    test_is_current_token_a_type();
    test_expect_token();
    test_expect_ctype_token();
    
}