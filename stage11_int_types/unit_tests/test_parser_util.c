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

void test_create_binary_op_node() {
    ASTNode * lhs = create_int_literal_node(42);
    ASTNode * rhs = create_var_decl_node("a", &CTYPE_INT_T, NULL);
    BinaryOperator op = BINOP_ADD;
    ASTNode * node = create_binary_op_node(lhs, op, rhs);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node ast type is AST_BINARY_EXPR", node->type == AST_BINARY_EXPR);
    TEST_ASSERT("Verify correct op", node->binary.op == op);
}

void test_create_var_decl_node() {
    ASTNode * node = create_var_decl_node("a", &CTYPE_INT_T, NULL);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node ast type is AST_VAR_DECL", node->type == AST_VAR_DECL);
    TEST_ASSERT("Verify node has correct identifier", strcmp(node->var_decl.name, "a") == 0);
    TEST_ASSERT("Verity node has NULL initializer", node->var_decl.init_expr == NULL);
    TEST_ASSERT("Verify node has correct ctype", ctype_equal(node->ctype, &CTYPE_INT_T));
}

void test_create_translation_unit_node() {

}

void test_create_if_else_statement_node() {
    ASTNode * lhs = create_var_decl_node("a", &CTYPE_INT_T, NULL);
    ASTNode * rhs = create_int_literal_node(42);
    BinaryOperator op = BINOP_LT;
    ASTNode * condExpr = create_binary_op_node(lhs, op, rhs);

    ASTNode * thenStatement = create_return_statement_node(create_int_literal_node(1));
    ASTNode * elseStatement = create_return_statement_node(create_int_literal_node(2));

    ASTNode * node = create_if_else_statement_node(condExpr, thenStatement, elseStatement);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_IF_STMT", node->type == AST_IF_STMT);
}

void test_create_while_statement_node() {
    ASTNode * lhs = create_var_decl_node("a", &CTYPE_INT_T, NULL);
    ASTNode * rhs = create_int_literal_node(42);
    BinaryOperator op = BINOP_LT;
    ASTNode * condExpr = create_binary_op_node(lhs, op, rhs);
    ASTNode * bodyStatement = create_return_statement_node(create_int_literal_node(1));

    ASTNode * node = create_while_statement_node(condExpr, bodyStatement);
    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_WHILE_STMT", node->type == AST_WHILE_STMT);

}

void test_create_ast_labeled_statement_node() {
    const char * label = "end";
    ASTNode * body = create_return_statement_node(create_int_literal_node(2));

    ASTNode * labeled_statement = create_ast_labeled_statement_node(label, body);

    TEST_ASSERT("Verify node is not null", labeled_statement);
    TEST_ASSERT("Verify node is type AST_LABELED_STMT", labeled_statement->type == AST_LABELED_STMT); 
    TEST_ASSERT("Verify label is correct", strcmp(labeled_statement->labeled_stmt.label, label) == 0);
}

void test_is_next_token_assignment() {
    tokenlist * tokens = malloc(sizeof(tokenlist));
    tokenlist_init(tokens, free_token);
 
    tokenlist_append(tokens, make_int_token("42", 1, 1));
    tokenlist_append(tokens, make_token(TOKEN_ASSIGN, "=", 1,1));
    tokenlist_append(tokens, make_token(TOKEN_PLUS_EQUAL, "+=", 1, 1));
    tokenlist_append(tokens, make_token(TOKEN_MINUS_EQUAL, "-=", 1, 1));
    tokenlist_append(tokens, make_eof_token(1,1));

    ParserContext * parserContext = create_parser_context(tokens);
    // next token should be = which is an assignment token
    TEST_ASSERT("Verifying next token is assignment", is_next_token_assignment(parserContext) == true);
    advance_parser(parserContext);

    // next token should be += which is an assignment token
    TEST_ASSERT("Verifying next token is an assignment", is_next_token_assignment(parserContext) == true);
    advance_parser(parserContext);

    // next token should be -= which is an assignment token
    TEST_ASSERT("Verifying next token is an assignment", is_next_token_assignment(parserContext) == true);
    advance_parser(parserContext);

    // next token should be EOF which is not an assignment
    TEST_ASSERT("Verifying next token is not an assignment", is_next_token_assignment(parserContext) == false);

}

int main() {
    RUN_TEST(test_create_int_literal_node);
    RUN_TEST(test_create_var_decl_node);
    RUN_TEST(test_create_unary_node);
    RUN_TEST(test_create_binary_op_node);
    RUN_TEST(test_create_translation_unit_node);
    RUN_TEST(test_create_if_else_statement_node);
    RUN_TEST(test_create_while_statement_node);
    RUN_TEST(test_is_next_token_assignment);
    RUN_TEST(test_create_ast_labeled_statement_node);
}