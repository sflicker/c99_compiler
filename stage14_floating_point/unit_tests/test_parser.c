#include <stdio.h>
#include <stdlib.h>

#include "list_util.h"
#include "token.h"
#include "tokenizer.h"
#include "ast.h"
#include "parser_context.h"
#include "parser.h"
#include "parser_util.h"
#include "test_assert.h"
#include "ast_printer.h"
#include "error.h"
#include "parser_util.h"
#include "c_type_printer.h"
#include "parse_expression.h"

const char * current_test = NULL;

void test_return_statement() {
    ASTNode * expected = create_return_statement_node(
        create_binary_node(
            create_int_literal_node(20),
            BINOP_ADD,
            create_int_literal_node(22)
        )
    );
    printf("Expected\n");
    print_ast(expected, 0);

    tokenlist * tokens = tokenize("return 20+22;");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_statement(ctx);
    printf("Actual\n");
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}

void test_expression_statement() {
    ASTNode * expected = create_expression_statement_node(
        create_binary_node(
            create_var_ref_node("a"),
            BINOP_ASSIGNMENT,
            create_binary_node(
                create_var_ref_node("b"),
                BINOP_MUL,
                create_function_call_node("myfunc", NULL)
            )
        )
    );

    printf("Expected\n");
    print_ast(expected, 0);

    tokenlist * tokens = tokenize("a = b * myfunc();");
    ParserContext * ctx = create_parser_context(tokens);
    ASTNode * actual = parse_statement(ctx);
    printf("Actual\n");
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}

void test_for_statement() {
    ASTNode * init_expr = create_declaration_node(&CTYPE_INT_T);
    ASTNode * init_expr_var = create_var_decl_node(
            "i",
            &CTYPE_INT_T,
            create_int_literal_node(0));
    ASTNode_list_append(init_expr->declaration.init_declarator_list, init_expr_var);

    ASTNode * cond_expr = create_binary_node(
        create_var_ref_node("i"),
        BINOP_LT,
        create_int_literal_node(10));
    ASTNode * update_expr = create_unary_node(UNARY_POST_INC,
        create_var_ref_node("i"));
    ASTNode_list * stmts = create_node_list();
    ASTNode * inc = create_expression_statement_node(
        create_unary_node(
            UNARY_POST_INC,
            create_var_ref_node("sum")
        )
    );
    ASTNode_list_append(stmts, inc);

    ASTNode * block = create_block_node(stmts);
    ASTNode * expected = create_for_statement_node(
        init_expr,
        cond_expr,
        update_expr,
        block);
    printf("Expected\n");
    print_ast(expected, 0);

    tokenlist * tokens = tokenize("for (int i=0;i<10;i++) { sum++; }");
    ParserContext * ctx = create_parser_context(tokens);
    ASTNode * actual = parse_statement(ctx);
    printf("Actual\n");
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}

void test_for_statement__empty() {

    ASTNode_list * stmts = create_node_list();
    ASTNode * rtn = create_return_statement_node(create_int_literal_node(42));

    ASTNode_list_append(stmts, rtn);

    ASTNode * block = create_block_node(stmts);

    ASTNode * expected = create_for_statement_node(
    NULL,
    NULL,
    NULL,
    block);
    printf("Expected\n");
    print_ast(expected, 0);

    tokenlist * tokens = tokenize("for (;;) { return 42; }");
    ParserContext * ctx = create_parser_context(tokens);
    ASTNode * actual = parse_statement(ctx);
    printf("Actual\n");
    print_ast(actual, 0);
}

void test_for_statement__only_cond() {
    // ASTNode * init_expr = create_var_decl_node(
    //         "i",
    //         &CTYPE_INT_T,
    //         create_int_literal_node(0));
    ASTNode * cond_expr = create_binary_node(
        create_var_ref_node("i"),
        BINOP_LT,
        create_int_literal_node(10));
    // ASTNode * update_expr = create_unary_node(UNARY_POST_INC,
    //     create_var_ref_node("i"));
    ASTNode_list * stmts = create_node_list();
    ASTNode * inc = create_expression_statement_node(
        create_unary_node(
            UNARY_POST_INC,
            create_var_ref_node("sum")
        )
    );
    ASTNode_list_append(stmts, inc);

    ASTNode * block = create_block_node(stmts);
    ASTNode * expected = create_for_statement_node(
        /*init_expr*/NULL,
        cond_expr,
        /*update_expr*/NULL,
        block);
    printf("Expected\n");
    print_ast(expected, 0);

    tokenlist * tokens = tokenize("for (;i<10;) { sum++; }");
    ParserContext * ctx = create_parser_context(tokens);
    ASTNode * actual = parse_statement(ctx);
    printf("Actual\n");
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);

    free_ast(actual);
}

void test_for_statement__only_update() {
    ASTNode * update_expr = create_unary_node(UNARY_POST_INC,
        create_var_ref_node("i"));
    ASTNode_list * stmts = create_node_list();
    ASTNode * inc = create_expression_statement_node(
        create_unary_node(
            UNARY_POST_INC,
            create_var_ref_node("sum")
        )
    );
    ASTNode_list_append(stmts, inc);

    ASTNode * block = create_block_node(stmts);
    ASTNode * expected = create_for_statement_node(
        /*init_expr*/NULL,
        /*cond_expr*/NULL,
        update_expr,
        block);
    printf("Expected\n");
    print_ast(expected, 0);

    tokenlist * tokens = tokenize("for (;;i++) { sum++; }");
    ParserContext * ctx = create_parser_context(tokens);
    ASTNode * actual = parse_statement(ctx);
    printf("Actual\n");
    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}


void test_array_with_initializer() {
    tokenlist * tokens = tokenize("{ int a[3] = {2, 4, 6}; }");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * astNode = parse_block(ctx);

    print_ast(astNode, 0);

    TEST_ASSERT("Verify node is not NULL", astNode != NULL);
    TEST_ASSERT("Verify node is a BLOOK", astNode->type == AST_BLOCK_STMT);
    TEST_ASSERT("Verify block has one child node", astNode->block.statements->count == 1);

    ASTNode * array_decl_stmt = astNode->block.statements->head->value;
    TEST_ASSERT("Verify array decl is not NULL", array_decl_stmt != NULL);
    TEST_ASSERT("Verify array decl has correct type", array_decl_stmt->type == AST_DECLARATION_STMT);
    ASTNode_list * init_declarator_list = array_decl_stmt->declaration.init_declarator_list;
    ASTNode * initializer = init_declarator_list->head->value->var_decl.init_expr;
    TEST_ASSERT("Verify initializer is not NULL", initializer != NULL);
    TEST_ASSERT("Verify initializer is of type AST_INITIALIZER_LIST", initializer->type == AST_INITIALIZER_LIST);
    TEST_ASSERT("Verify initializer has 3 items", initializer->initializer_list.items->count == 3);
    TEST_ASSERT("Verify value of first item is 2",initializer->initializer_list.items->head->value->int_value == 2);

}

int main() {

    RUN_TEST(test_return_statement);
    RUN_TEST(test_expression_statement);
    RUN_TEST(test_for_statement);
    RUN_TEST(test_for_statement__empty);
    RUN_TEST(test_for_statement__only_cond);
    RUN_TEST(test_for_statement__only_update);
    RUN_TEST(test_array_with_initializer);

}