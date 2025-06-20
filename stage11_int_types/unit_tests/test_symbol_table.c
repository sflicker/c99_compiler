//
// Created by scott on 6/15/25.
//

#include "test_assert.h"
#include "symbol_table.h"

const char * current_test = NULL;

void test_init_symbol_table() {
    init_symbol_table();
}

void test_scope() {
    init_symbol_table();
    enter_scope();
    exit_scope();
}

void test_add_function_symbol__no_args() {
    init_symbol_table();
    const char * func_name = "main";
    CType * return_type = &CTYPE_INT_T;
    int param_count = 0;
    CTypePtr_list * param_types = NULL;

    FunctionSymbol * function_symbol = add_function_symbol(func_name, return_type, param_count, param_types);

    TEST_ASSERT("Verifying function_symbol is not null", function_symbol != NULL);
    TEST_ASSERT("Verifying function name is 'main'", strcmp(function_symbol->name, func_name) == 0);
    TEST_ASSERT("Verifying function return type is CTYPE_INT_T", ctype_equals(function_symbol->return_ctype,  return_type));
    TEST_ASSERT("Verifying function parameter count is 0", function_symbol->param_count == 0);
    TEST_ASSERT("Verifying function parameter types is null", function_symbol->param_types == NULL);

    free_function_symbol(function_symbol);
}

void test_add_function_symbol__with_args() {
    init_symbol_table();
    const char * func_name = "main";
    CType * return_type = &CTYPE_INT_T;
    int param_count = 2;

    CTypePtr_list * param_types = malloc(sizeof(CTypePtr_list));
    CTypePtr_list_init(param_types, free_ctype);

    CTypePtr_list_append(param_types, &CTYPE_INT_T);
    CTypePtr_list_append(param_types, &CTYPE_INT_T);

    CTypePtr_list * expected_param_types = malloc(sizeof(CTypePtr_list));
    CTypePtr_list_init(expected_param_types, free_ctype);

    CTypePtr_list_append(expected_param_types, &CTYPE_INT_T);
    CTypePtr_list_append(expected_param_types, &CTYPE_INT_T);


    FunctionSymbol * function_symbol = add_function_symbol(func_name, return_type, param_count, param_types);

    TEST_ASSERT("Verifying function_symbol is not null", function_symbol != NULL);
    TEST_ASSERT("Verifying function name is 'main'", strcmp(function_symbol->name, func_name) == 0);
    TEST_ASSERT("Verifying function return type is CTYPE_INT_T", ctype_equals(function_symbol->return_ctype,  return_type));
    TEST_ASSERT("Verifying function parameter count is 2", function_symbol->param_count == param_count);
    TEST_ASSERT("Verifying function parameter types is not null", function_symbol->param_types != NULL);
    TEST_ASSERT("Verifying function parameter lists are equal", ctype_lists_equal(function_symbol->param_types, expected_param_types));

    free_function_symbol(function_symbol);

}

void test_add_symbol() {
    init_symbol_table();
    enter_scope();
    Symbol * symbol = add_symbol("a", &CTYPE_INT_T, NULL);

    TEST_ASSERT("Verifying symbol is not null", symbol != NULL);
    TEST_ASSERT("Verifying symbol name is 'a'", strcmp(symbol->name, symbol->name) == 0);
    TEST_ASSERT("Verifying symbol has correct type", ctype_equals(symbol->ctype, &CTYPE_INT_T));

}

void test_lookup_symbol() {
    init_symbol_table();
    enter_scope();
    add_symbol("a", &CTYPE_INT_T, NULL);
    Symbol * symbol = lookup_symbol("a");

    TEST_ASSERT("Verifying symbol is not null", symbol != NULL);
    TEST_ASSERT("Verifying symbol name is 'a'", strcmp(symbol->name, symbol->name) == 0);
    TEST_ASSERT("Verifying symbol has correct type", ctype_equals(symbol->ctype, &CTYPE_INT_T));

}

void test_add_symbol_in_scope() {
    init_symbol_table();
    enter_scope();
    add_symbol("a", &CTYPE_INT_T, NULL);

    enter_scope();
    add_symbol("b", &CTYPE_INT_T, NULL);
}

void test_lookup_symbol_in_scope() {
    init_symbol_table();
    enter_scope();
    add_symbol("a", &CTYPE_INT_T, NULL);

    enter_scope();
    add_symbol("b", &CTYPE_INT_T, NULL);

    Symbol * a_symbol = lookup_symbol("a");
    Symbol * b_symbol = lookup_symbol("b");

    TEST_ASSERT("Verifying a symbol is not null", a_symbol != NULL);
    TEST_ASSERT("Verifying b symbol is not null", b_symbol != NULL);

}

void test_add_nested_symbols() {
    init_symbol_table();
    enter_scope();
    add_symbol("a", &CTYPE_INT_T, NULL);

    enter_scope();
    add_symbol("a", &CTYPE_LONG_T, NULL);

    enter_scope();
    add_symbol("a", &CTYPE_SHORT_T, NULL);


}

void test_nested_symbol_lookup() {
    init_symbol_table();
    enter_scope();
    add_symbol("a", &CTYPE_INT_T, NULL);

    enter_scope();
    add_symbol("a", &CTYPE_LONG_T, NULL);

    enter_scope();
    add_symbol("a", &CTYPE_SHORT_T, NULL);

    Symbol * a_symbol = lookup_symbol("a");

    TEST_ASSERT("Verifying symbol is not null", a_symbol != NULL);
    TEST_ASSERT("Verifying symbol has correct type", ctype_equals(a_symbol->ctype, &CTYPE_SHORT_T));

    exit_scope();

    a_symbol = lookup_symbol("a");

    TEST_ASSERT("Verifying symbol is not null", a_symbol != NULL);
    TEST_ASSERT("Verifying symbol has correct type", ctype_equals(a_symbol->ctype, &CTYPE_LONG_T));

    exit_scope();

    a_symbol = lookup_symbol("a");

    TEST_ASSERT("Verifying symbol is not null", a_symbol != NULL);
    TEST_ASSERT("Verifying symbol has correct type", ctype_equals(a_symbol->ctype, &CTYPE_INT_T));

}

int main() {
    RUN_TEST(test_init_symbol_table);
    RUN_TEST(test_scope);
    RUN_TEST(test_add_function_symbol__no_args);
    RUN_TEST(test_add_function_symbol__with_args);
    RUN_TEST(test_add_symbol);
    RUN_TEST(test_lookup_symbol);
    RUN_TEST(test_add_symbol_in_scope);
    RUN_TEST(test_lookup_symbol_in_scope);
    RUN_TEST(test_add_nested_symbols);
    RUN_TEST(test_nested_symbol_lookup);
}