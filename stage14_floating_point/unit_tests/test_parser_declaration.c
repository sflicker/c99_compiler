//
// Created by scott on 8/16/25.
//

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

void test_declarator__scalar_var() {
    tokenlist * tokens = tokenize("int a");
    ParserContext * ctx = create_parser_context(tokens);

//    char * name = NULL;
    CType * base_type = parse_type_specifier(ctx);
    printf("BaseType\n");
    print_c_type(base_type, 0);
    Declarator * declarator = make_declarator();
    declarator->type = base_type;
    declarator = parse_declarator(ctx, declarator);
    printf("FullType\n");
    print_c_type(declarator->type, 0);

    TEST_ASSERT("Verify base_type and full_type are the same", ctype_equals(base_type, declarator->type));
    TEST_ASSERT("Verify Correct name", strcmp("a", declarator->name) == 0);
}

void test_declarator__array_var() {
    tokenlist * tokens = tokenize("int a[10]");

    ParserContext * ctx = create_parser_context(tokens);

    CType * base_type = parse_type_specifier(ctx);
    Declarator * declarator = make_declarator();
    declarator->type = base_type;
    declarator = parse_declarator(ctx, declarator);

    TEST_ASSERT("Verify base_type and full_type are not the same", !ctype_equals(base_type, declarator->type));
    TEST_ASSERT("Verify Correct name", strcmp("a", declarator->name) == 0);
    TEST_ASSERT("Verify full_type has Array kind", declarator->type->kind == CTYPE_ARRAY);
    TEST_ASSERT("Verify full_type has Array len of 10", declarator->type->array_len == 10);
    TEST_ASSERT("Verify full_type has Array size of 40", declarator->type->size == 40);
    TEST_ASSERT("Verify full_type has int base_type", declarator->type->base_type->kind == CTYPE_INT);

}

void test_declarator__array_multi_var() {
    tokenlist * tokens = tokenize("int a[10][5]");

    ParserContext * ctx = create_parser_context(tokens);

    Declarator * declarator = make_declarator(ctx);
    CType * base_type = parse_type_specifier(ctx);
    declarator->type = base_type;
    print_c_type(base_type, 0);
    declarator = parse_declarator(ctx, declarator /*, &name, NULL, NULL*/);
    print_c_type(declarator->type, 0);

    const char * name = declarator->name;

    TEST_ASSERT("Verify base_type and full_type are not the same", !ctype_equals(base_type, declarator->type));
    TEST_ASSERT("Verify Correct name", strcmp("a", name) == 0);
    TEST_ASSERT("Verify full_type has Array kind", declarator->type->kind == CTYPE_ARRAY);
    TEST_ASSERT("Verify outer array has correct size", declarator->type->array_len == 10);
    TEST_ASSERT("Verify full_type has Array base_type", declarator->type->base_type->kind == CTYPE_ARRAY);
    TEST_ASSERT("Verify inner array has correct size", declarator->type->base_type->array_len == 5);
    TEST_ASSERT("Verify full_type.base.base has int_type", declarator->type->base_type->base_type->kind == CTYPE_INT);

}

void test_declarator__function_no_args() {
    tokenlist * tokens = tokenize("int main()");

    ParserContext * ctx = create_parser_context(tokens);

    CType * base_type = parse_type_specifier(ctx);
    Declarator * declarator = make_declarator(ctx);
    declarator->type = base_type;
    declarator = parse_declarator(ctx, declarator /*, &name, &params, &func_type*/);

    TEST_ASSERT("Verify base_type and return_type.base_type are the same", ctype_equals(base_type, declarator->type->base_type));
    TEST_ASSERT("Verify full_type has the correct kind", declarator->type->kind == CTYPE_FUNCTION);
    TEST_ASSERT("Verify Correct name", strcmp("main", declarator->name) == 0);

}

void test_declarator__function_with_body_but_no_args() {
    tokenlist * tokens = tokenize("int main() { return 42; }");

    ParserContext * ctx = create_parser_context(tokens);
    Declarator * declarator = make_declarator(ctx);
    CType * base_type = parse_type_specifier(ctx);
    declarator->type = base_type;
    declarator = parse_declarator(ctx, declarator/*, &name, &params, &func_type*/);

    TEST_ASSERT("Verify base_type and full_type.base_type are the same", ctype_equals(base_type, declarator->type->base_type));
    TEST_ASSERT("Verify Correct name", strcmp("main", declarator->name) == 0);
    TEST_ASSERT("Verify full_type has the correct kind", declarator->type->kind == CTYPE_FUNCTION);
    TEST_ASSERT("Verify current char is {", is_current_token(ctx, TOKEN_LBRACE));
}

void test_declarator__scalar_var_pointer() {
    tokenlist * tokens = tokenize("int * a");
    ParserContext * ctx = create_parser_context(tokens);

    Declarator * declarator = make_declarator(ctx);
    CType * base_type = parse_type_specifier(ctx);
    declarator->type = base_type;
    declarator = parse_declarator(ctx, declarator/*, &name, NULL, NULL*/);

    TEST_ASSERT("Verify base_type and full_type not are the same", !ctype_equals(base_type, declarator->type));
    TEST_ASSERT("Verify full_type has pointer kind", declarator->type->kind == CTYPE_PTR);
    TEST_ASSERT("Verify full_type.base_type is base_type", ctype_equals(declarator->type->base_type, base_type));
    TEST_ASSERT("Verify Correct name", strcmp("a", declarator->name) == 0);

}

void test_declarator__scalar_var_pointer_to_pointer() {
    tokenlist * tokens = tokenize("int ** a");
    ParserContext * ctx = create_parser_context(tokens);

    CType * base_type = parse_type_specifier(ctx);
    Declarator * declarator = make_declarator(ctx);
    printf("BaseType:\n");
    print_c_type(base_type, 0);
    declarator->type = base_type;
    declarator = parse_declarator(ctx, declarator/*, &name, NULL, NULL*/);
    printf("FullType:\n");
    print_c_type(declarator->type, 0);

    TEST_ASSERT("Verify base_type and full_type not are the same", !ctype_equals(base_type, declarator->type));
    TEST_ASSERT("Verify full_type has pointer kind", declarator->type->kind == CTYPE_PTR);
    TEST_ASSERT("Verity full_type base has pointer kind", declarator->type->base_type->kind == CTYPE_PTR);
    TEST_ASSERT("Verify full_type.base_type.base_type is base_type", ctype_equals(declarator->type->base_type->base_type, base_type));
    TEST_ASSERT("Verify Correct name", strcmp("a", declarator->name) == 0);

}

void test_declarator__array_of_int_pointers() {
    tokenlist * tokens = tokenize("int * a[10]");

    ParserContext * ctx = create_parser_context(tokens);
    Declarator * declarator = make_declarator(ctx);

    CType * base_type = parse_type_specifier(ctx);
    declarator->type = base_type;
    printf("BaseType:\n");
    print_c_type(base_type, 0);
    declarator = parse_declarator(ctx, declarator /*, &name, NULL, NULL*/);
    printf("FullType:\n");
    print_c_type(declarator->type, 0);

    TEST_ASSERT("Verify base_type and full_type not are the same", !ctype_equals(base_type, declarator->type));
    TEST_ASSERT("Verify full_type has array kind", declarator->type->kind == CTYPE_ARRAY);
    TEST_ASSERT("Verity full_type.base has pointer kind", declarator->type->base_type->kind == CTYPE_PTR);
    TEST_ASSERT("Verify full_type.base_type.base_type is int", declarator->type->base_type->base_type->kind == CTYPE_INT);
    TEST_ASSERT("Verify Correct name", strcmp("a", declarator->name) == 0);

}

void test_declarator__function_with_pointer_return() {
    tokenlist * tokens = tokenize("int * myfunc()");

    ParserContext * ctx = create_parser_context(tokens);
    Declarator * declarator = make_declarator(ctx);
    CType * base_type = parse_type_specifier(ctx);
    declarator->type = base_type;
    declarator = parse_declarator(ctx, declarator /*, &name, &params, &func_type*/);

    TEST_ASSERT("Verify base_type and return_type not are the same", !ctype_equals(base_type, declarator->type));
//    TEST_ASSERT("Verify func_type has the correct kind", func_type->kind == CTYPE_FUNCTION);
    TEST_ASSERT("Verify return_type has function kind", declarator->type->kind == CTYPE_FUNCTION);
    TEST_ASSERT("Verify return_type.base_type has pointer kind", declarator->type->base_type->kind == CTYPE_PTR);
    TEST_ASSERT("Verify return_type.base.base has int kind", declarator->type->base_type->base_type->kind == CTYPE_INT);
    TEST_ASSERT("Verify Correct name", strcmp("myfunc", declarator->name) == 0);

}

void test_declaration__with_comma() {
    tokenlist * tokens = tokenize("{ int a, b; }");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node = parse_block(ctx);

    TEST_ASSERT("Verifying node is not null", node != NULL);
    TEST_ASSERT("Verifying node is a block", node->type == AST_BLOCK_STMT);

    ASTNode * declaration_node = node->block.statements->head->value;
    TEST_ASSERT("Verifying declaration_node is not null", declaration_node != NULL);
    TEST_ASSERT("Verifying declaration_node is a declaration", declaration_node->type == AST_DECLARATION_STMT);

    TEST_ASSERT("Verifying declaration_node has correct number of init_declaration", declaration_node->declaration.init_declarator_list->count == 2);

    ASTNode * init_declaration = ASTNode_list_get(declaration_node->declaration.init_declarator_list, 0);
    TEST_ASSERT("Verifying init_declaration is not null", init_declaration != NULL);
    TEST_ASSERT("Verifying init_declaration has correct type", init_declaration->type == AST_VAR_DECL);
    TEST_ASSERT("Verifying init_declaration has correct ctype", init_declaration->ctype == &CTYPE_INT_T);
    TEST_ASSERT("Verifying init_declaration has correct identifier", strcmp(init_declaration->var_decl.name, "a") == 0);

    init_declaration = ASTNode_list_get(declaration_node->declaration.init_declarator_list, 1);
    TEST_ASSERT("Verifying init_declaration is not null", init_declaration != NULL);
    TEST_ASSERT("Verifying init_declaration has correct type", init_declaration->type == AST_VAR_DECL);
    TEST_ASSERT("Verifying init_declaration has correct ctype", init_declaration->ctype == &CTYPE_INT_T);
    TEST_ASSERT("Verifying init_declaration has correct identifier", strcmp(init_declaration->var_decl.name, "b") == 0);

}

int main() {

    RUN_TEST(test_declarator__scalar_var);
    RUN_TEST(test_declarator__array_var);
    RUN_TEST(test_declarator__array_multi_var);
    RUN_TEST(test_declarator__function_no_args);
    RUN_TEST(test_declarator__function_with_body_but_no_args);
    RUN_TEST(test_declarator__scalar_var_pointer);
    RUN_TEST(test_declarator__scalar_var_pointer_to_pointer);
    RUN_TEST(test_declarator__array_of_int_pointers);
    RUN_TEST(test_declarator__function_with_pointer_return);
    RUN_TEST(test_declaration__with_comma);
}