#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "test_assert.h"

#include "token.h"
#include "tokenizer.h"
#include "tokenizer_context.h"

const char * current_test = NULL;

void basic_test() {

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

    tokenlist * tokens = tokenize(program_text);
    tokenlist_cursor * cursor=malloc(sizeof(tokenlist_cursor));
    tokenlist_cursor_init(cursor, tokens);
    int count = 0;
    char buffer[128];
    while(tokenlist_cursor_has_next(cursor)) {
        snprintf(buffer, sizeof(buffer), "Verifing Token is of type: %s", token_type_name(expected_tokens[count]));
        TEST_ASSERT(buffer, expected_tokens[count] == cursor->current->value->type);
        count++;
        tokenlist_cursor_next(cursor);
    }
}

void test_match_keyword(const char * text, TokenType expectedType) {
    char msg_buf[128];
    snprintf(msg_buf, sizeof(msg_buf), "Attempting to match keyword: %s", text);

    TokenizerContext * ctx = init_tokenizer_context(text);
    Token * token = match_keyword(ctx, text);
    TEST_ASSERT(msg_buf, expectedType == token->type);
    
    free_tokenizer_context(ctx);
}

void test_match_keywords() {
    test_match_keyword("int", TOKEN_INT);
    test_match_keyword("for", TOKEN_FOR);
    test_match_keyword("switch", TOKEN_SWITCH);
    test_match_keyword("while", TOKEN_WHILE);
    test_match_keyword("case", TOKEN_CASE);
    test_match_keyword("do", TOKEN_DO);
    test_match_keyword("if", TOKEN_IF);
    test_match_keyword("else", TOKEN_ELSE);
}

int main() {
    RUN_TEST(basic_test);
    RUN_TEST(test_match_keywords);
}

