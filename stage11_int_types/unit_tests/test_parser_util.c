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
#include "ctypes.h"

const char * current_test = NULL;

void test_create_int_literal_node() {
    ASTNode * node = create_int_literal_node(42);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify int value is 42", node->int_value == 42);
    TEST_ASSERT("Verify ast type is AST_INT_LITERAL", node->type == AST_INT_LITERAL);
    TEST_ASSERT("Verify ctype is NULL", node->ctype == NULL);
}

void test_create_unary_node__plus() {
    ASTNode * operand = create_var_decl_node("a", &CTYPE_INT_T, NULL);
    ASTNode * node = create_unary_node(UNARY_PLUS, operand);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node ast type is AST_UNARY_EXPR", node->type == AST_UNARY_EXPR);

}

void test_create_var_decl_node() {
    ASTNode * node = create_var_decl_node("a", &CTYPE_INT_T, NULL);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node ast type is AST_VAR_DECL", node->type == AST_VAR_DECL);
    TEST_ASSERT("Verify node has correct identifier", strcmp(node->var_decl.name, "a") == 0);
    TEST_ASSERT("Verity node has NULL initializer", node->var_decl.init_expr == NULL);
    TEST_ASSERT("Verify node has correct ctype", ctype_equal(node->ctype, &CTYPE_INT_T));
}

int main() {
    RUN_TEST(test_create_int_literal_node);
    RUN_TEST(test_create_var_decl_node);
    RUN_TEST(test_create_unary_node__plus);
}