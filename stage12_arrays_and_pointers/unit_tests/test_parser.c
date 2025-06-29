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

    free_ast(expected);
    free_ast(actual);

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

    free_ast(expected);
    free_ast(actual);

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

    free_ast(expected);
    free_ast(actual);
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

    free_ast(expected);
    free_ast(actual);

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

    free_ast(expected);
    free_ast(actual);
}

void test_parse_cast_expression() {
    ASTNode * expected = create_cast_expr_node(
        &CTYPE_INT_T,
        create_int_literal_node(10));

    tokenlist * tokens = tokenize("(int)10");
    ParserContext * ctx = create_parser_context(tokens);
    ASTNode * actual = parse_cast_expression(ctx);

    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}

void test_parse_multiplicative__multi() {
    ASTNode * expected = create_binary_node(
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

    free_ast(expected);
    free_ast(actual);

}

void test_parse_multiplicative__div() {
    ASTNode * expected = create_binary_node(
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

    free_ast(expected);
    free_ast(actual);

}

void test_parse_multiplicative__mod() {
    ASTNode * expected = create_binary_node(
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

    free_ast(expected);
    free_ast(actual);

}

void test_parse_additive_expression__add() {
    ASTNode * expected = create_binary_node(
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

    free_ast(expected);
    free_ast(actual);
}

void test_parse_additive_expression__sub() {
    ASTNode * expected = create_binary_node(
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

    free_ast(expected);
    free_ast(actual);
}

void test_parse_relational_expression__gt() {
    ASTNode * expected = create_binary_node(
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

    free_ast(expected);
    free_ast(actual);

}

void test_parse_relational_expression__ge() {
    ASTNode * expected = create_binary_node(
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

    free_ast(expected);
    free_ast(actual);

}

void test_parse_relational_expression__lt() {
    ASTNode * expected = create_binary_node(
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

    free_ast(expected);
    free_ast(actual);

}

void test_parse_relational_expression__le() {

    ASTNode * expected = create_binary_node(
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

    free_ast(expected);
    free_ast(actual);
}

void test_parse_constant_expression__pass() {
    // setup
    tokenlist * tokens = tokenize("7");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_constant_expression(ctx);

    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is not NULL", actual != NULL);
    TEST_ASSERT("Verifying error did not occur", !error_occurred());

    free_ast(actual);
}

void test_parse_constant_expression__fail() {
    // setup
    tokenlist * tokens = tokenize("a++");
    ParserContext * ctx = create_parser_context(tokens);

    set_error_exit_on_error_enabled(false);

    ASTNode * actual = parse_constant_expression(ctx);

    print_ast(actual, 0);
    TEST_ASSERT("Verifying node is NULL", actual == NULL);
    TEST_ASSERT("Verifying error occurred", error_occurred());
    TEST_ASSERT("Verifying error message is present", strlen(error_message()) > 0);

    free_ast(actual);

    set_error_exit_on_error_enabled(true);
}

void test_parse_equality_expression__eq() {
    ASTNode * expected = create_binary_node(
        create_var_ref_node("a"),
        BINOP_EQ,
        create_var_ref_node("b"));

    // setup
    tokenlist * tokens = tokenize("a == b");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_equality_expression(ctx);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_ast(expected);
    free_ast(actual);
}

void test_parse_equality_expression__ne() {
    ASTNode * expected = create_binary_node(
        create_var_ref_node("a"),
        BINOP_NE,
        create_var_ref_node("b"));

    // setup
    tokenlist * tokens = tokenize("a != b");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_equality_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));

    free_ast(expected);
    free_ast(actual);
}

void test_parse_logical_and() {
    ASTNode * expected = create_binary_node(
        create_var_ref_node("a"),
        BINOP_LOGICAL_AND,
        create_var_ref_node("b"));

    tokenlist * tokens = tokenize("a && b");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_logical_and(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}

void test_parse_logical_or() {
    ASTNode * expected = create_binary_node(
        create_var_ref_node("a"),
        BINOP_LOGICAL_OR,
        create_var_ref_node("b"));

    tokenlist * tokens = tokenize("a || b");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_logical_or(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);
}

void test_parse_assignment_expression__assignment() {
    ASTNode * expected = create_binary_node(
        create_var_ref_node("a"),
        BINOP_ASSIGNMENT,
        create_var_ref_node("b"));

    tokenlist * tokens = tokenize("a = b");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_assignment_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}

void test_parse_assignment_expression__add_assign() {
    ASTNode * expected = create_binary_node(
        create_var_ref_node("a"),
        BINOP_COMPOUND_ADD_ASSIGN,
        create_var_ref_node("b"));

    tokenlist * tokens = tokenize("a += b");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_assignment_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}

void test_parse_assignment_expression__sub_assign() {
    ASTNode * expected = create_binary_node(
        create_var_ref_node("a"),
        BINOP_COMPOUND_SUB_ASSIGN,
        create_var_ref_node("b"));

    tokenlist * tokens = tokenize("a -= b");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_assignment_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}

void test_parse_expression__1() {
    ASTNode * expected = create_binary_node(
    create_var_ref_node("a"),
    BINOP_ASSIGNMENT,
    create_binary_node(
        create_var_ref_node("b"),
        BINOP_ADD,
        create_binary_node(
            create_var_ref_node("c"),
            BINOP_MUL,
            create_function_call_node("myfunc", NULL)
            )
        )
    );

    tokenlist * tokens = tokenize("a = b + c * myfunc()");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_expression(ctx);
    print_ast(actual, 0);

    bool equal = ast_equal(expected, actual);
    TEST_ASSERT("Verifying node is correct", equal);
    free_ast(expected);
    free_ast(actual);

}

void test_parse_expression__2() {
    ASTNode * expected = create_binary_node(
    create_var_ref_node("a"),
    BINOP_ASSIGNMENT,
    create_binary_node(
        create_binary_node(
            create_var_ref_node("b"),
            BINOP_MUL,
            create_var_ref_node("c")),
        BINOP_ADD,
        create_function_call_node("myfunc", NULL)
        )
    );

    tokenlist * tokens = tokenize("a = b * c + myfunc()");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * actual = parse_expression(ctx);
    print_ast(actual, 0);

    TEST_ASSERT("Verifying node is correct", ast_equal(expected, actual));
    free_ast(expected);
    free_ast(actual);

}

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
    ASTNode * init_expr = create_var_decl_node(
            "i",
            &CTYPE_INT_T,
            create_int_literal_node(0));
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
    // ASTNode * init_expr = create_var_decl_node(
    //     "i",
    //     &CTYPE_INT_T,
    //     create_int_literal_node(0));
    // ASTNode * cond_expr = create_binary_node(
    //     create_var_ref_node("i"),
    //     BINOP_LT,
    //     create_int_literal_node(10));
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

void test_declarator__scalar_var() {
    tokenlist * tokens = tokenize("int a");
    ParserContext * ctx = create_parser_context(tokens);

    char * name = NULL;
    CType * base_type = parse_type_specifier(ctx);
    printf("BaseType\n");
    print_c_type(base_type, 0);
    CType * full_type = parse_declarator(ctx, base_type, &name, NULL, NULL);
    printf("FullType\n");
    print_c_type(full_type, 0);

    TEST_ASSERT("Verify base_type and full_type are the same", ctype_equals(base_type, full_type));
    TEST_ASSERT("Verify Correct name", strcmp("a", name) == 0);
}

void test_declarator__array_var() {
    tokenlist * tokens = tokenize("int a[10]");

    ParserContext * ctx = create_parser_context(tokens);

    char * name = NULL;
    CType * base_type = parse_type_specifier(ctx);
    CType * full_type = parse_declarator(ctx, base_type, &name, NULL, NULL);

    TEST_ASSERT("Verify base_type and full_type are not the same", !ctype_equals(base_type, full_type));
    TEST_ASSERT("Verify Correct name", strcmp("a", name) == 0);
    TEST_ASSERT("Verify full_type has Array kind", full_type->kind == CTYPE_ARRAY);
    TEST_ASSERT("Verify full_type has Array len of 10", full_type->array_len == 10);
    TEST_ASSERT("Verify full_type has Array size of 40", full_type->size == 40);
    TEST_ASSERT("Verify full_type has int base_type", full_type->base->kind == CTYPE_INT);

}

void test_declarator__array_multi_var() {
    tokenlist * tokens = tokenize("int a[10][10]");

    ParserContext * ctx = create_parser_context(tokens);

    char * name = NULL;
    CType * base_type = parse_type_specifier(ctx);
    print_c_type(base_type, 0);
    CType * full_type = parse_declarator(ctx, base_type, &name, NULL, NULL);
    print_c_type(full_type, 0);

    TEST_ASSERT("Verify base_type and full_type are not the same", !ctype_equals(base_type, full_type));
    TEST_ASSERT("Verify Correct name", strcmp("a", name) == 0);
    TEST_ASSERT("Verify full_type has Array kind", full_type->kind == CTYPE_ARRAY);
    TEST_ASSERT("Verify full_type has Array base_type", full_type->base->kind == CTYPE_ARRAY);
    TEST_ASSERT("Verify full_type.base.base has int_type", full_type->base->base->kind == CTYPE_INT);

}

void test_declarator__function_no_args() {
    tokenlist * tokens = tokenize("int main()");

    ParserContext * ctx = create_parser_context(tokens);

    char * name = NULL;
    ASTNode_list * params = NULL;
    CType * func_type = NULL;
    CType * base_type = parse_type_specifier(ctx);
    CType * return_type = parse_declarator(ctx, base_type, &name, &params, &func_type);

    TEST_ASSERT("Verify base_type and return_type are the same", ctype_equals(base_type, return_type));
    TEST_ASSERT("Verify func_type has the correct kind", func_type->kind == CTYPE_FUNCTION);
    TEST_ASSERT("Verify Correct name", strcmp("main", name) == 0);

}

void test_declarator__function_with_body_but_no_args() {
    tokenlist * tokens = tokenize("int main() { return 42; ");

    ParserContext * ctx = create_parser_context(tokens);

    char * name = NULL;
    ASTNode_list * params = NULL;
    CType * func_type = NULL;
    CType * base_type = parse_type_specifier(ctx);
    CType * full_type = parse_declarator(ctx, base_type, &name, &params, &func_type);

    TEST_ASSERT("Verify base_type and full_type are the same", ctype_equals(base_type, full_type));
    TEST_ASSERT("Verify Correct name", strcmp("main", name) == 0);
    TEST_ASSERT("Verify func_type has the correct kind", func_type->kind == CTYPE_FUNCTION);
    TEST_ASSERT("Verify current char is {", is_current_token(ctx, TOKEN_LBRACE));
}

void test_declarator__scalar_var_pointer() {
    tokenlist * tokens = tokenize("int * a");
    ParserContext * ctx = create_parser_context(tokens);

    char * name = NULL;
    CType * base_type = parse_type_specifier(ctx);
    CType * full_type = parse_declarator(ctx, base_type, &name, NULL, NULL);

    TEST_ASSERT("Verify base_type and full_type not are the same", !ctype_equals(base_type, full_type));
    TEST_ASSERT("Verify full_type has pointer kind", full_type->kind == CTYPE_PTR);
    TEST_ASSERT("Verify full_type.base_type is base_type", ctype_equals(full_type->base, base_type));
    TEST_ASSERT("Verify Correct name", strcmp("a", name) == 0);

}

void test_declarator__scalar_var_pointer_to_pointer() {
    tokenlist * tokens = tokenize("int ** a");
    ParserContext * ctx = create_parser_context(tokens);

    char * name = NULL;
    CType * base_type = parse_type_specifier(ctx);
    printf("BaseType:\n");
    print_c_type(base_type, 0);
    CType * full_type = parse_declarator(ctx, base_type, &name, NULL, NULL);
    printf("FullType:\n");
    print_c_type(full_type, 0);

    TEST_ASSERT("Verify base_type and full_type not are the same", !ctype_equals(base_type, full_type));
    TEST_ASSERT("Verify full_type has pointer kind", full_type->kind == CTYPE_PTR);
    TEST_ASSERT("Verity full_type base has pointer kind", full_type->base->kind == CTYPE_PTR);
    TEST_ASSERT("Verify full_type.base_type.base_type is base_type", ctype_equals(full_type->base->base, base_type));
    TEST_ASSERT("Verify Correct name", strcmp("a", name) == 0);

}

void test_declarator__array_of_int_pointers() {
    tokenlist * tokens = tokenize("int * a[10]");

    ParserContext * ctx = create_parser_context(tokens);

    char * name = NULL;
    CType * base_type = parse_type_specifier(ctx);
    printf("BaseType:\n");
    print_c_type(base_type, 0);
    CType * full_type = parse_declarator(ctx, base_type, &name, NULL, NULL);
    printf("FullType:\n");
    print_c_type(full_type, 0);

    TEST_ASSERT("Verify base_type and full_type not are the same", !ctype_equals(base_type, full_type));
    TEST_ASSERT("Verify full_type has array kind", full_type->kind == CTYPE_ARRAY);
    TEST_ASSERT("Verity full_type.base has pointer kind", full_type->base->kind == CTYPE_PTR);
    TEST_ASSERT("Verify full_type.base_type.base_type is int", full_type->base->base->kind == CTYPE_INT);
    TEST_ASSERT("Verify Correct name", strcmp("a", name) == 0);

}

void test_declarator__function_with_pointer_return() {
    tokenlist * tokens = tokenize("int * myfunc()");

    ParserContext * ctx = create_parser_context(tokens);

    char * name = NULL;
    ASTNode_list * params = NULL;
    CType * func_type = NULL;
    CType * base_type = parse_type_specifier(ctx);
    CType * return_type = parse_declarator(ctx, base_type, &name, &params, &func_type);

    TEST_ASSERT("Verify base_type and return_type not are the same", !ctype_equals(base_type, return_type));
    TEST_ASSERT("Verify func_type has the correct kind", func_type->kind == CTYPE_FUNCTION);
    TEST_ASSERT("Verify return_type has pointer kind", return_type->kind == CTYPE_PTR);
    TEST_ASSERT("Verify return_type.base has int kind", return_type->base->kind == CTYPE_INT);
    TEST_ASSERT("Verify Correct name", strcmp("myfunc", name) == 0);

}

int main() {
    RUN_TEST(test_parse_primary__int_literal);
    RUN_TEST(test_parse_primary__parens);
    RUN_TEST(test_parse_primary__function_call);
    RUN_TEST(test_parse_primary__function_call__with_args);
    RUN_TEST(test_parse_primary__variable_ref);
    RUN_TEST(test_parse_constant_expression__pass);
    RUN_TEST(test_parse_constant_expression__fail);
    RUN_TEST(test_parse_postfix_expression__inc);
    RUN_TEST(test_parse_postfix_expression__dec);
    RUN_TEST(test_parse_unary_expression__plus);
    RUN_TEST(test_parse_unary_expression__negate);
    RUN_TEST(test_parse_unary_expression__not);
    RUN_TEST(test_parse_unary_expression__increment);
    RUN_TEST(test_parse_unary_expression__decrement);
    RUN_TEST(test_parse_cast_expression);
    RUN_TEST(test_parse_multiplicative__multi);
    RUN_TEST(test_parse_multiplicative__div);
    RUN_TEST(test_parse_multiplicative__mod);
    RUN_TEST(test_parse_additive_expression__add);
    RUN_TEST(test_parse_additive_expression__sub);
    RUN_TEST(test_parse_relational_expression__gt);
    RUN_TEST(test_parse_relational_expression__ge);
    RUN_TEST(test_parse_relational_expression__lt);
    RUN_TEST(test_parse_relational_expression__le);
    RUN_TEST(test_parse_equality_expression__eq);
    RUN_TEST(test_parse_equality_expression__ne);
    RUN_TEST(test_parse_logical_and);
    RUN_TEST(test_parse_logical_or);
    RUN_TEST(test_parse_assignment_expression__assignment);
    RUN_TEST(test_parse_assignment_expression__add_assign);
    RUN_TEST(test_parse_assignment_expression__sub_assign);
    RUN_TEST(test_parse_expression__1);
    RUN_TEST(test_parse_expression__2);
    RUN_TEST(test_return_statement);
    RUN_TEST(test_expression_statement);
    RUN_TEST(test_for_statement);
    RUN_TEST(test_for_statement__empty);
    RUN_TEST(test_for_statement__only_cond);
    RUN_TEST(test_for_statement__only_update);
    RUN_TEST(test_declarator__scalar_var);
    RUN_TEST(test_declarator__array_var);
    RUN_TEST(test_declarator__array_multi_var);
    RUN_TEST(test_declarator__function_no_args);
    RUN_TEST(test_declarator__function_with_body_but_no_args);
    RUN_TEST(test_declarator__scalar_var_pointer);
    RUN_TEST(test_declarator__scalar_var_pointer_to_pointer);
    RUN_TEST(test_declarator__array_of_int_pointers);
    RUN_TEST(test_declarator__function_with_pointer_return);

}