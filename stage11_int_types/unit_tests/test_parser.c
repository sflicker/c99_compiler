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

const char * current_test = NULL;

void test_parse_primary__int_literal() {
    tokenlist * tokens = tokenize("42");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node = parse_primary(ctx);
    print_ast(node, 0);

    TEST_ASSERT("Verifying node is of type AST_INT_LITERAL", node->type == AST_INT_LITERAL);
    TEST_ASSERT("Verifying node has correct int_value", node->int_value == 42);
    TEST_ASSERT("Verifying node ctype is NULL", node->ctype == NULL);
    free_parser_context(ctx);
    tokenlist_free(tokens);
}

void test_parse_primary__parens() {
    tokenlist * tokens = tokenize("(42)");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node = parse_primary(ctx);
    print_ast(node, 0);

    TEST_ASSERT("Verifying node is of type AST_INT_LITERAL", node->type == AST_INT_LITERAL);
    TEST_ASSERT("Verifying node has correct int_value", node->int_value == 42);
    TEST_ASSERT("Verifying node ctype is NULL", node->ctype == NULL);
    free_parser_context(ctx);
    tokenlist_free(tokens);
}

void test_parse_primary__function_call() {
    tokenlist * tokens = tokenize("myfunc()");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node = parse_primary(ctx);
    print_ast(node, 0);

    TEST_ASSERT("Verifying node is of type AST_FUNCTION_CALL", node->type == AST_FUNCTION_CALL);
    TEST_ASSERT("Verifying correct name", strcmp(node->function_call.name, "myfunc") == 0);
    TEST_ASSERT("Verifying argument list is empty", node->function_call.arg_list == NULL);
    TEST_ASSERT("Verifying node ctype is NULL", node->ctype == NULL);

    free_parser_context(ctx);
    tokenlist_free(tokens);
}

void test_parse_primary__function_call__with_args() {
    tokenlist * tokens = tokenize("myfunc(1,2,3,4)");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node = parse_primary(ctx);
    print_ast(node, 0);

    TEST_ASSERT("Verifying node is of type AST_FUNCTION_CALL", node->type == AST_FUNCTION_CALL);
    TEST_ASSERT("Verifying correct name", strcmp(node->function_call.name, "myfunc") == 0);
    TEST_ASSERT("Verifying argument list contains 4 args", node->function_call.arg_list->count == 4);
    TEST_ASSERT("Verifying node ctype is NULL", node->ctype == NULL);

    free_parser_context(ctx);
    tokenlist_free(tokens);
}

void test_parse_primary__variable_ref() {
    tokenlist * tokens = tokenize("a");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node = parse_primary(ctx);
    print_ast(node, 0);

    TEST_ASSERT("Verifying node is of type AST_VAR_REF", node->type == AST_VAR_REF);
    TEST_ASSERT("Verifying correct name", strcmp(node->function_call.name, "a") == 0);
    TEST_ASSERT("Verifying node ctype is NULL", node->ctype == NULL);

    free_parser_context(ctx);
    tokenlist_free(tokens);
}

void test_parse_postfix_expression__inc() {
    // setup
    tokenlist * tokens = tokenize("a++");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * node = parse_postfix_expression(ctx);
    print_ast(node, 0);


    // verify
    TEST_ASSERT("Verifying node is of type AST_UNARY_POST_INC", node->type == AST_UNARY_EXPR);
    TEST_ASSERT("Verifying op is of type UNARY_POST_INC", node->unary.op == UNARY_POST_INC);
    TEST_ASSERT("Verifying Operand is correct type", node->unary.operand->type == AST_VAR_REF);
    TEST_ASSERT("Verifying Operand has correct name", strcmp(node->unary.operand->var_ref.name, "a") == 0);
    TEST_ASSERT("Verifying node ctype is NULL", node->ctype == NULL);

    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);

}

void test_parse_postfix_expression__dec() {
    // setup
    tokenlist * tokens = tokenize("a--");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * node = parse_postfix_expression(ctx);
    print_ast(node, 0);

    // verify
    TEST_ASSERT("Verifying node os of type AST_UNARY_POST_DEC", node->type == AST_UNARY_EXPR);
    TEST_ASSERT("Verifying Operand is correct type", node->unary.operand->type == AST_VAR_REF);
    TEST_ASSERT("Verifying Operand has correct name", strcmp(node->unary.operand->var_ref.name, "a") == 0);
    TEST_ASSERT("Verifying node ctype is NULL", node->ctype == NULL);

    // cleanup
    free_parser_context(ctx);
    tokenlist_free(tokens);

}

void test_parse_unary_expression__plus() {
    // setup
    tokenlist * tokens = tokenize("+a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * node = parse_unary_expression(ctx);
    print_ast(node, 0);

}

void test_parse_unary_expression__negate() {
    // setup
    tokenlist * tokens = tokenize("-a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * node = parse_unary_expression(ctx);
    print_ast(node, 0);

}

void test_parse_unary_expression__not() {
    // setup
    tokenlist * tokens = tokenize("!a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * node = parse_unary_expression(ctx);
    print_ast(node, 0);

}

void test_parse_unary_expression__increment() {
    // setup
    tokenlist * tokens = tokenize("++a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * node = parse_unary_expression(ctx);
    print_ast(node, 0);

}

void test_parse_unary_expression__decrement() {
    // setup
    tokenlist * tokens = tokenize("--a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * node = parse_unary_expression(ctx);
    print_ast(node, 0);

}

int main() {
    RUN_TEST(test_parse_primary__int_literal);
    RUN_TEST(test_parse_primary__parens);
    RUN_TEST(test_parse_primary__function_call);
    RUN_TEST(test_parse_primary__function_call__with_args);
    RUN_TEST(test_parse_primary__variable_ref);
    RUN_TEST(test_parse_postfix_expression__inc);
    RUN_TEST(test_parse_postfix_expression__dec);
    RUN_TEST(test_parse_unary_expression__plus);
    RUN_TEST(test_parse_unary_expression__negate);
    RUN_TEST(test_parse_unary_expression__not);
    RUN_TEST(test_parse_unary_expression__increment);
    RUN_TEST(test_parse_unary_expression__decrement);

}