#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "test_assert.h"
#include "list_util.h"
#include "token.h"

void test_init_token_list() {
    tokenlist * list = malloc(sizeof(tokenlist));
    init_token_list(list);
    TEST_ASSERT("Verifying list head is NULL", list->head == NULL);
    TEST_ASSERT("Verifying list tail is NULL", list->tail == NULL);
    cleanup_token_list(list);
}

void test_add_token() {
    tokenlist * list = malloc(sizeof(tokenlist));
    init_token_list(list);
    TEST_ASSERT("Verifying list has 0 elements", list->count == 0);
    Token * token = make_eof_token(1,1);
    add_token(list, token);
    TEST_ASSERT("Verifying list has 1 element", list->count == 1);
}

void test_make_token() {
    Token * token = make_token(TOKEN_RETURN, "return", 1,1);
    TEST_ASSERT("Verifying token has correct type", token->type == TOKEN_RETURN);
    TEST_ASSERT("Verifying token has correct text", strcmp("return", token->text)==0);
    TEST_ASSERT("Verifying token has correct text length", strlen("return") == token->length);
    TEST_ASSERT("Verifying token has correct positions", token->line == 1 && token->col == 1);
}

void test_make_int_token() {
    Token * token = make_int_token("42", 1,1);
    TEST_ASSERT("Verifying token has correct type", token->type == TOKEN_INT_LITERAL);
    TEST_ASSERT("Verifying token has correct text", strcmp("42", token->text)==0);
    TEST_ASSERT("Verifying token has correct text length", strlen("42") == token->length);
    TEST_ASSERT("Verifying token has correct int value", token->int_value == 42);
    TEST_ASSERT("Verifying token has correct positions", token->line == 1 && token->col == 1);
}

void test_token_type_name() {
    TEST_ASSERT("Verifying token has correct name text", strcmp("EOF", token_type_name(TOKEN_EOF))==0);
}

int main() {
    printf("Starting test_token\n");
    test_init_token_list();
    test_add_token();
    test_make_token();
    test_make_int_token();
    test_token_type_name();
    printf("Finished test_token\n");
}