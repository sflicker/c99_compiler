#include <stdio.h>
#include <stdlib.h>

#include "list_util.h"
#include "token.h"
#include "tokenizer.h"
#include "ast.h"
#include "parser_context.h"
#include "parser.h"
#include "test_assert.h"
#include "ast_printer.h"
#include "parser_util.h"

const char * current_test = NULL;

void test_create_int_literal_node() {
    ASTNode * node = create_int_literal_node(42);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify int value is 42", node->int_value == 42);
    TEST_ASSERT("Verify ast type is AST_INT_LITERAL", node->type == AST_INT_LITERAL);
    TEST_ASSERT("Verify ctype is NULL", node->ctype == NULL);
}

void test_create_unary_node__plus() {

}


int main() {
    RUN_TEST(test_create_int_literal_node);
    RUN_TEST(test_create_unary_node__plus);
}