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
#include "c_type.h"

const char * current_test = NULL;

void test_create_translation_unit_node() {
    
    ASTNode * rtn_stmt_node = create_return_statement_node(create_int_literal_node(2));
    ASTNode_list * statements = create_node_list();
    ASTNode_list_append(statements, rtn_stmt_node);
    
    ASTNode * body = create_block_node(statements);

    ASTNode * func = create_function_declaration_node("main", &CTYPE_INT_T, NULL, body, false);

    ASTNode_list * functions = create_node_list();
    ASTNode_list_append(functions, func);

    ASTNode_list * globals = create_node_list();
    ASTNode_list_append(globals, create_var_decl_node("a", &CTYPE_INT_T, NULL));

    ASTNode * node = create_translation_unit_node(functions, globals);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node type is AST_TRANSLATION_UNIT", node->type == AST_TRANSLATION_UNIT);
    TEST_ASSERT("Verify node has 1 function", node->translation_unit.count == 1);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);
    TEST_ASSERT("Verify node has one global", node->translation_unit.globals->count == 1);

    free_ast(node);
}

void test_create_unary_node() {
    ASTNode * operand = create_var_decl_node("a", &CTYPE_INT_T, NULL);
    ASTNode * node = create_unary_node(UNARY_PLUS, operand);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node ast type is AST_UNARY_EXPR", node->type == AST_UNARY_EXPR);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_binary_node() {
    ASTNode * lhs = create_int_literal_node(42);
    ASTNode * rhs = create_var_decl_node("a", &CTYPE_INT_T, NULL);
    ASTNode * node = create_binary_node(lhs, BINOP_ADD, rhs);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node ast type is AST_BINARY_EXPR", node->type == AST_BINARY_EXPR);
    TEST_ASSERT("Verify correct op", node->binary.op == BINOP_ADD);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_if_else_statement_node() {
    ASTNode * lhs = create_var_decl_node("a", &CTYPE_INT_T, NULL);
    ASTNode * rhs = create_int_literal_node(42);
    ASTNode * condExpr = create_binary_node(lhs, BINOP_LT, rhs);

    ASTNode * thenStatement = create_return_statement_node(create_int_literal_node(1));
    ASTNode * elseStatement = create_return_statement_node(create_int_literal_node(2));

    ASTNode * node = create_if_else_statement_node(condExpr, thenStatement, elseStatement);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_IF_STMT", node->type == AST_IF_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_while_statement_node() {
    ASTNode * lhs = create_var_decl_node("a", &CTYPE_INT_T, NULL);
    ASTNode * rhs = create_int_literal_node(42);
    ASTNode * condExpr = create_binary_node(lhs, BINOP_LT, rhs);
    ASTNode * bodyStatement = create_return_statement_node(create_int_literal_node(1));

    ASTNode * node = create_while_statement_node(condExpr, bodyStatement);
    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_WHILE_STMT", node->type == AST_WHILE_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_do_while_statement() {
    ASTNode * lhs = create_var_decl_node("a", &CTYPE_INT_T, NULL);
    ASTNode * rhs = create_int_literal_node(42);
    ASTNode * condExpr = create_binary_node(lhs, BINOP_LT, rhs);
    ASTNode * bodyStatement = create_return_statement_node(create_int_literal_node(1));

    ASTNode * node = create_do_while_statement(bodyStatement, condExpr);
    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_DO_WHILE_STMT", node->type == AST_DO_WHILE_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_ast_labeled_statement_node() {
    const char * label = "end";
    ASTNode * body = create_return_statement_node(create_int_literal_node(2));

    ASTNode * node = create_ast_labeled_statement_node(label, body);

    TEST_ASSERT("Verify node is not null", node);
    TEST_ASSERT("Verify node is type AST_LABELED_STMT", node->type == AST_LABELED_STMT); 
    TEST_ASSERT("Verify label is correct", strcmp(node->labeled_stmt.label, label) == 0);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_ast_case_statement_node() {
    ASTNode * constExpression = create_int_literal_node(42);
    ASTNode * stmt = create_return_statement_node(create_int_literal_node(42));
    ASTNode * node = create_ast_case_statement_node(constExpression, stmt);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_CASE_STMT", node->type == AST_CASE_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);
    free_ast(node);
}

void test_create_ast_default_statement_node() {
    ASTNode * stmt = create_return_statement_node(create_int_literal_node(42));
    ASTNode * node = create_ast_default_statement_node(stmt);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_DEFAULT_STMT", node->type == AST_DEFAULT_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);
    free_ast(node);
}

void test_create_goto_statement() {
    const char * label = "end";
    ASTNode * node = create_goto_statement(label);

    TEST_ASSERT("Verify node is not null", node);
    TEST_ASSERT("Verify node is type AST_GOTO_STMT", node->type == AST_GOTO_STMT); 
    TEST_ASSERT("Verify label is correct", strcmp(node->goto_stmt.label, label) == 0);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_switch_statement_node() {
    ASTNode * constExpression = create_int_literal_node(42);
    ASTNode * stmt = create_return_statement_node(create_int_literal_node(42));
    ASTNode * caseStmt = create_ast_case_statement_node(constExpression, stmt);

    stmt = create_return_statement_node(create_int_literal_node(42));
    ASTNode * defaultStmt = create_ast_default_statement_node(stmt);

    ASTNode_list * statements = create_node_list();
    ASTNode_list_append(statements, caseStmt);
    ASTNode_list_append(statements, defaultStmt);

    ASTNode * body = create_block_node(statements);

    ASTNode * expr = create_var_ref_node("a");

    ASTNode * node = create_switch_statement(expr, body);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_SWITCH_STMT", node->type == AST_SWITCH_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_break_statement_node() {
    ASTNode * node = create_break_statement_node();

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_BREAK_STMT", node->type == AST_BREAK_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_continue_statement_node() {
    ASTNode * node = create_continue_statement_node();

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_CONTINUE_STMT", node->type == AST_CONTINUE_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);

}

void test_create_int_literal_node() {
    ASTNode * node = create_int_literal_node(42);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify int value is 42", node->int_value == 42);
    TEST_ASSERT("Verify ast type is AST_INT_LITERAL", node->type == AST_INT_LITERAL);
    TEST_ASSERT("Verify ctype is CTYPE_INT_T", node->ctype == &CTYPE_INT_T);

    free_ast(node);
}

void test_create_function_call_node_no_args() {
    const char * funcname = "sum";

    ASTNode * node = create_function_call_node(funcname, NULL);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_FUNCTION_CALL", node->type == AST_FUNCTION_CALL_EXPR);
    TEST_ASSERT("Verify function label is correct", strcmp(node->var_ref.name, funcname) == 0);
    TEST_ASSERT("Verify arg list is NULL", node->function_call.arg_list == NULL);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_function_call_node_with_args() {
    const char * funcname = "sum";
    ASTNode_list * arg_list = create_node_list();
    ASTNode_list_append(arg_list, create_int_literal_node(42));

    ASTNode * node = create_function_call_node(funcname, arg_list);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_FUNCTION_CALL", node->type == AST_FUNCTION_CALL_EXPR);
    TEST_ASSERT("Verify function label is correct", strcmp(node->var_ref.name, funcname) == 0);
    TEST_ASSERT("Verify arg list contains 1 arg", node->function_call.arg_list->count == 1);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_var_ref_node() {
    const char * var_name = "a";

    ASTNode * node = create_var_ref_node(var_name);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is type AST_VAR_REF", node->type == AST_VAR_REF_EXPR);
    TEST_ASSERT("Verify var name is correct", strcmp(node->var_ref.name, var_name) == 0);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_var_decl_node() {
    ASTNode * node = create_var_decl_node("a", &CTYPE_INT_T, NULL);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node ast type is AST_VAR_DECL", node->type == AST_VAR_DECL);
    TEST_ASSERT("Verify node has correct identifier", strcmp(node->var_decl.name, "a") == 0);
    TEST_ASSERT("Verity node has NULL initializer", node->var_decl.init_expr == NULL);
    TEST_ASSERT("Verify node has correct ctype", ctype_equals(node->ctype, &CTYPE_INT_T));

    free_ast(node);

}

void test_create_for_statement_node() {
    ASTNode * init_expr = create_binary_node(
        create_var_ref_node("a"),
        BINOP_ASSIGNMENT,
        create_int_literal_node(0)
        );
    ASTNode * cond_expr = create_binary_node(
        create_var_ref_node("a"),
        BINOP_LT,
        create_int_literal_node(10)
    );
    ASTNode * update_expr = create_unary_node(
        UNARY_POST_INC,
        create_var_ref_node("a")
        );
    ASTNode_list * body_list = create_node_list();
        ASTNode_list_append(body_list, create_unary_node(UNARY_POST_INC,
            create_var_ref_node("counter")
            ));
    ASTNode * body = create_block_node(body_list);

    ASTNode * node = create_for_statement_node(init_expr, cond_expr, update_expr, body);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node ast type is AST_FOR_STMT", node->type == AST_FOR_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(init_expr);
}

void test_create_function_declaration_node__declaration_only() {
    const char * label = "myfunc";
    CType * return_type = &CTYPE_INT_T;
    CType_list * param_types = NULL;
    ASTNode_list * param_list = NULL;
    CType * func_type = make_function_type(return_type, param_types);

    ASTNode * body = NULL;
    bool declaration_only = true;

    ASTNode * node = create_function_declaration_node(label, func_type, param_list, body, declaration_only);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is AST_FUNCTION_DECL", node->type == AST_FUNCTION_DECL);
    TEST_ASSERT("Verify correct identifier", strcmp(label, node->function_decl.name) == 0);
    TEST_ASSERT("Verify declaration only", node->function_decl.declaration_only);
    TEST_ASSERT("Verify node ctype is CType_int", ctype_equals(node->ctype, &CTYPE_INT_T));

    free_ast(node);
}

void test_create_function_declaration_node__with_body() {
    const char * label = "main";

    CType * return_type = &CTYPE_INT_T;
    CType_list * param_types = NULL;
    CType * func_type = make_function_type(return_type, param_types);


    ASTNode * rtn_stmt_node = create_return_statement_node(create_int_literal_node(2));
    ASTNode_list * statements = create_node_list();
    ASTNode_list_append(statements, rtn_stmt_node);
    
    ASTNode * body = create_block_node(statements);

    ASTNode * node = create_function_declaration_node(label, func_type, NULL, body, false);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is AST_FUNCTION_DECL", node->type == AST_FUNCTION_DECL);
    TEST_ASSERT("Verify correct identifier", strcmp(label, node->function_decl.name) == 0);
    TEST_ASSERT("Verify is not declaration only", !node->function_decl.declaration_only);
    TEST_ASSERT("Verify function body contains 1 statement", node->function_decl.body->block.count == 1);
    TEST_ASSERT("Verify node ctype is CTYPE_INT", ctype_equals(node->ctype, &CTYPE_INT_T));

    free_ast(node);
}

void test_create_function_declaration_node__with_body_and_param_list() {
    const char * label = "main";
    const char * param_label = "a";

    CType * return_type = &CTYPE_INT_T;
    CType_list * param_types = NULL;
    CType * func_type = make_function_type(return_type, param_types);

    ASTNode * rtn_stmt_node = create_return_statement_node(create_int_literal_node(2));
    ASTNode_list * statements = create_node_list();
    ASTNode_list_append(statements, rtn_stmt_node);

    ASTNode * param_a = create_var_decl_node(param_label, &CTYPE_INT_T, NULL);
    ASTNode_list * param_list = malloc(sizeof(ASTNode_list));
    ASTNode_list_init(param_list, free_ast);
    ASTNode_list_append(param_list, param_a);
    
    ASTNode * body = create_block_node(statements);

    ASTNode * node = create_function_declaration_node(label, func_type, param_list, body, false);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is AST_FUNCTION_DECL", node->type == AST_FUNCTION_DECL);
    TEST_ASSERT("Verify correct identifier", strcmp(label, node->function_decl.name) == 0);
    TEST_ASSERT("Verify is not declaration only", !node->function_decl.declaration_only);
    TEST_ASSERT("Verify function body contains 1 statement", node->function_decl.body->block.count == 1);
    TEST_ASSERT("Verify function contains 1 param", node->function_decl.param_list->count == 1);
    TEST_ASSERT("Verify node ctype is CTYPE_INT", ctype_equals(node->ctype, &CTYPE_INT_T));

    free_ast(node);
}

void test_create_return_statement_node() {
    ASTNode * node = create_return_statement_node(create_int_literal_node(2));

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is AST_RETURN_STMT", node->type == AST_RETURN_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_expression_statement_node() {
    ASTNode * node =
        create_expression_statement_node(
        create_binary_node(
            create_var_ref_node("a"),
            BINOP_ASSIGNMENT,
            create_int_literal_node(3)
            )
        );
    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is AST_EXPRESSION_STMT", node->type == AST_EXPRESSION_STMT);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);
}

void test_create_block_node() {
    ASTNode * rtn_stmt_node = create_return_statement_node(create_int_literal_node(2));
    ASTNode_list * statements = create_node_list();
    ASTNode_list_append(statements, rtn_stmt_node);
    
    ASTNode * node = create_block_node(statements);
    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is AST_BLOCK", node->type == AST_BLOCK_STMT);
    TEST_ASSERT("Verify block has 1 statement", node->block.count == 1);
    TEST_ASSERT("Verify node ctype is null", node->ctype == NULL);

    free_ast(node);

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

void test_create_initializer_list() {
    ASTNode_list * expressions = create_node_list();
    ASTNode_list_append(expressions, create_int_literal_node(4));
    ASTNode_list_append(expressions, create_int_literal_node(6));
    ASTNode_list_append(expressions, create_int_literal_node(8));

    ASTNode * node = create_initializer_list(expressions);

    TEST_ASSERT("Verify node is not null", node != NULL);
    TEST_ASSERT("Verify node is AST_INITIALIZER_LIST", node->type == AST_INITIALIZER_LIST);
    TEST_ASSERT("Verify node ctype is not null", node->ctype != NULL);
    TEST_ASSERT("Verify node ctype is CTYPE_INT", ctype_equals(node->ctype, &CTYPE_INT_T));
    TEST_ASSERT("Verify node initializer ctype is Array", node->initializer_list.initializer_type->kind == CTYPE_ARRAY);
    TEST_ASSERT("Verify node contains 3 items", node->initializer_list.items->count == 3);
    TEST_ASSERT("Verify first value is 4", node->initializer_list.items->head->value->int_value == 4);
}

int main() {
    RUN_TEST(test_create_translation_unit_node);
    RUN_TEST(test_create_unary_node);
    RUN_TEST(test_create_binary_node);
    RUN_TEST(test_create_if_else_statement_node);
    RUN_TEST(test_create_while_statement_node);
    RUN_TEST(test_create_ast_labeled_statement_node);
    RUN_TEST(test_create_ast_case_statement_node);
    RUN_TEST(test_create_ast_default_statement_node);
    RUN_TEST(test_create_goto_statement);
    RUN_TEST(test_create_do_while_statement);
    RUN_TEST(test_create_switch_statement_node);
    RUN_TEST(test_create_break_statement_node);
    RUN_TEST(test_create_continue_statement_node);
    RUN_TEST(test_create_int_literal_node);
    RUN_TEST(test_create_function_call_node_no_args);
    RUN_TEST(test_create_function_call_node_with_args);
    RUN_TEST(test_create_var_ref_node);
    RUN_TEST(test_create_var_decl_node);
    RUN_TEST(test_create_for_statement_node);
    RUN_TEST(test_create_function_declaration_node__declaration_only);
    RUN_TEST(test_create_function_declaration_node__with_body);
    RUN_TEST(test_create_function_declaration_node__with_body_and_param_list);
    RUN_TEST(test_create_return_statement_node);
    RUN_TEST(test_create_expression_statement_node);
    RUN_TEST(test_create_block_node);
    RUN_TEST(test_create_initializer_list);

    RUN_TEST(test_is_next_token_assignment);
}