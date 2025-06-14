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

void test_parse_primary__int_literal() {
    tokenlist * tokens = tokenize("42");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node = parse_primary(ctx);
    print_ast(node, 0);

    TEST_ASSERT("Verifying node is of type AST_INT_LITERAL", node->type == AST_INT_LITERAL);
    TEST_ASSERT("Verifying node has correct int_value", node->int_value == 42);
    TEST_ASSERT("Verifying node ctype is CTYPE_INT_T", node->ctype == &CTYPE_INT_T);
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
    TEST_ASSERT("Verifying node ctype is CTYPE_INT_T", node->ctype == &CTYPE_INT_T);
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
    ASTNode * expected =
        create_unary_node(
            UNARY_PLUS,
            create_var_ref_node("a")
        );
    // setup
    tokenlist * tokens = tokenize("+a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_unary_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);

}

void test_parse_unary_expression__negate() {
    ASTNode * expected =
        create_unary_node(
            UNARY_NEGATE,
            create_var_ref_node("a")
        );

    // setup
    tokenlist * tokens = tokenize("-a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_unary_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);

}

void test_parse_unary_expression__not() {
    ASTNode * expected =
    create_unary_node(
        UNARY_NOT,
        create_var_ref_node("a")
    );
    // setup
    tokenlist * tokens = tokenize("!a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_unary_expression(ctx);
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);
}

void test_parse_unary_expression__increment() {
    ASTNode * expected =
    create_unary_node(
        UNARY_PRE_INC,
        create_var_ref_node("a")
    );

    // setup
    tokenlist * tokens = tokenize("++a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_unary_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);

}

void test_parse_unary_expression__decrement() {
    ASTNode * expected =
        create_unary_node(
            UNARY_PRE_DEC,
            create_var_ref_node("a")
    );
    // setup
    tokenlist * tokens = tokenize("--a");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_unary_expression(ctx);
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);
}

void test_parse_multiplicative__multi() {
    ASTNode * expected = create_binary_op_node(
        create_int_literal_node(1),
        BINOP_MUL,
        create_int_literal_node(2));
    // setup
    tokenlist * tokens = tokenize("1 * 2");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_multiplicative_expression(ctx);
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);

}

void test_parse_multiplicative__div() {
    ASTNode * expected = create_binary_op_node(
        create_int_literal_node(10),
        BINOP_DIV,
        create_int_literal_node(2));
    // setup
    tokenlist * tokens = tokenize("10 / 2");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_multiplicative_expression(ctx);
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);

}

void test_parse_multiplicative__mod() {
    ASTNode * expected = create_binary_op_node(
        create_int_literal_node(7),
        BINOP_MOD,
        create_int_literal_node(2));
    // setup
    tokenlist * tokens = tokenize("7 % 2");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_multiplicative_expression(ctx);
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);

}

void test_parse_additive_expression__add() {
    ASTNode * expected = create_binary_op_node(
        create_int_literal_node(7),
        BINOP_ADD,
        create_int_literal_node(2));
    // setup
    tokenlist * tokens = tokenize("7 + 2");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_additive_expression(ctx);
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);
}

void test_parse_additive_expression__sub() {
    ASTNode * expected = create_binary_op_node(
        create_int_literal_node(7),
        BINOP_SUB,
        create_int_literal_node(2));
    // setup
    tokenlist * tokens = tokenize("7 - 2");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_additive_expression(ctx);
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);
}

void parse_relational_expression__gt() {
    ASTNode * expected = create_binary_op_node(
        create_int_literal_node(7),
        BINOP_GT,
        create_int_literal_node(2));
    // setup
    tokenlist * tokens = tokenize("7 > 2");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_relational_expression(ctx);
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);

}

void parse_relational_expression__ge() {
    ASTNode * expected = create_binary_op_node(
        create_int_literal_node(7),
        BINOP_GE,
        create_int_literal_node(2));
    // setup
    tokenlist * tokens = tokenize("7 >= 2");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_relational_expression(ctx);
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);

}

void parse_relational_expression__lt() {
    ASTNode * expected = create_binary_op_node(
        create_int_literal_node(7),
        BINOP_LT,
        create_int_literal_node(2));
    // setup
    tokenlist * tokens = tokenize("7 < 2");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_relational_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);

}

void parse_relational_expression__le() {

    ASTNode * expected = create_binary_op_node(
        create_int_literal_node(7),
        BINOP_LE,
        create_int_literal_node(2));

    // setup
    tokenlist * tokens = tokenize("7 <= 2");
    ParserContext * ctx = create_parser_context(tokens);

    // test
    ASTNode * actual = parse_relational_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_astnode(expected);
    free_astnode(actual);
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
    RUN_TEST(test_parse_multiplicative__multi);
    RUN_TEST(test_parse_multiplicative__div);
    RUN_TEST(test_parse_multiplicative__mod);
    RUN_TEST(test_parse_additive_expression__add);
    RUN_TEST(test_parse_additive_expression__sub);
    RUN_TEST(parse_relational_expression__gt);
    RUN_TEST(parse_relational_expression__ge);
    RUN_TEST(parse_relational_expression__lt);
    RUN_TEST(parse_relational_expression__le);

}