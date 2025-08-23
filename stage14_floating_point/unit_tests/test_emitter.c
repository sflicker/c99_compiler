//
// Created by scott on 7/9/25.
//

#include <stdio.h>

#include "test_assert.h"
#include "token.h"
#include "tokenizer.h"
#include "analyzer.h"
#include "parser.h"
#include "parse_expression.h"
#include "parser_context.h"
#include "parser_util.h"
#include "ast_printer.h"
#include "symbol_table.h"
#include "analyzer_context.h"
#include "c_type.h"
#include "emitter.h"
#include "emitter_context.h"
#include "emitter_helpers.h"

const char * current_test = NULL;

typedef enum { EXPR, BLOCK, GLOBAL} PARSER_OP;

void run_emitter_test(char * c_fragment, char * expected, PARSER_OP op) {
    TEST_MSG("Test Fragment: ");
    TEST_MSG(c_fragment);
    tokenlist * tokens = tokenize(c_fragment);
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node;
    switch (op) {
        case EXPR:
            node = parse_expression(ctx);
            break;
        case BLOCK:
            node = parse_block(ctx);
            break;
        case GLOBAL:
            node = parse_translation_unit(ctx);
            break;
    }
    printf("\nAST After Parsing\n");
    print_ast(node, 0);
    free_parser_context(ctx);
    tokenlist_free(tokens);

    init_global_table();
    reset_size_and_offsets();
    AnalyzerContext * analyzer_context = analyzer_context_new();
    analyze(analyzer_context, node);
    analyzer_context_free(analyzer_context);

    printf("AST After Analyzer\n");
    print_ast(node, 0);

    char * buffer = NULL;
    size_t buffer_size = 0;

    FILE * memf = open_memstream(&buffer, &buffer_size);

    EmitterContext * emitter_context = create_emitter_context_from_fp(memf);

    // run codegen 'emit' test
    emit_tree_node(emitter_context, node);

    emitter_finalize(emitter_context);

    printf("Generated code:\n%s\n", buffer);
    char * normalizedOutput = calloc(strlen(buffer) + 1, 1);

    strip_comments_multiline(buffer, normalizedOutput);

    TEST_ASSERT_EQ_STR("Verifying generated code is correct", expected, normalizedOutput);

    free(normalizedOutput);
}


void test_emit_basic_expr() {
    TEST_MSG("Basic expression test");
    char * c_fragment = "42";
    char * expected =
        "mov eax, 42\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected, EXPR);
}

void test_emit_add_literals_expr() {
    TEST_MSG("add two literals test");
    char * c_fragment = "20 + 22";

    char * expected =
        "mov eax, 20\n"
        "push rax\n"
        "mov eax, 22\n"
        "push rax\n"
        "pop rcx\n"
        "pop rax\n"
        "movsxd rax, eax\n"
        "movsxd rcx, ecx\n"
        "add rax, rcx\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected, EXPR);

}

void test_emit_add_int_var_block() {
    TEST_MSG("add two literals test");
    char * c_fragment = "{"
                        "    int a = 1;"
                        "    int b = 2;"
                        "    int c = a + b;"
                        "}";

    char * expected =
        "mov eax, 1\n"
        "push rax\n"
        "lea rcx, [rbp-4]\n"
        "push rcx\n"
        "pop rcx\n"
        "pop rax\n"
        "mov DWORD [rcx], eax\n"
        "mov eax, 2\n"
        "push rax\n"
        "lea rcx, [rbp-8]\n"
        "push rcx\n"
        "pop rcx\n"
        "pop rax\n"
        "mov DWORD [rcx], eax\n"
        "mov eax, [rbp-4]\n"
        "push rax\n"
        "mov eax, [rbp-8]\n"
        "push rax\n"
        "pop rcx\n"
        "pop rax\n"
        "movsxd rax, eax\n"
        "movsxd rcx, ecx\n"
        "add rax, rcx\n"
        "push rax\n"
        "lea rcx, [rbp-12]\n"
        "push rcx\n"
        "pop rcx\n"
        "pop rax\n"
        "mov DWORD [rcx], eax\n";

    run_emitter_test(c_fragment, expected, BLOCK);

}

void test_emit_int_declaration() {
    TEST_MSG("declaration test");
    char * c_fragment =
        "{"
        "    int a = 1;"
        "}";

    char * expected = "mov eax, 1\n"
                      "push rax\n"
                      "lea rcx, [rbp-4]\n"
                      "push rcx\n"
                      "pop rcx\n"
                      "pop rax\n"
                      "mov DWORD [rcx], eax\n";

    run_emitter_test(c_fragment, expected, BLOCK);

}

void test_emit_multi_literals_expr() {
    TEST_MSG("multi literals test");
    char * c_fragment = "2 * 3";
    char * expected =
        "mov eax, 2\n"
        "push rax\n"
        "mov eax, 3\n"
        "push rax\n"
        "pop rcx\n"
        "pop rax\n"
        "imul eax, ecx\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected, EXPR);

}

void test_emit_literal_with_int_cast() {
    TEST_MSG("test int cast of literal");
    char * c_fragment = "(int)42";
    char * expected =
        "mov eax, 42\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected, EXPR);
}

void test_emit_literal_with_char_cast() {
    char * c_fragment = "(char)42";

    TEST_MSG("test char cast of literal");

    char * expected =
        "mov eax, 42\n"
        "push rax\n"
        "pop rax\n"
        "movsx eax, al\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected, EXPR);
}

void test_emit_literal_with_short_cast() {
    TEST_MSG("test short cast of literal");
    char * c_fragment = "(short)42";
    char * expected =
        "mov eax, 42\n"
        "push rax\n"
        "pop rax\n"
        "movsx eax, ax\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected, EXPR);
}

void test_emit_add_assign() {
    TEST_MSG("add assignment test");
    char * c_fragment = "{ \n"
                        "  int a = 41; \n"
                        "  a += 1; \n"
                        "}";

    char * expected = "mov eax, 41\n"
                      "push rax\n"
                      "lea rcx, [rbp-4]\n"
                      "push rcx\n"
                      "pop rcx\n"
                      "pop rax\n"
                      "mov DWORD [rcx], eax\n"
                      "lea rcx, [rbp-4]\n"
                      "push rcx\n"
                      "mov DWORD eax, [rcx]\n"
                      "push rax\n"
                      "mov eax, 1\n"
                      "push rax\n"
                      "pop rcx\n"
                      "pop rax\n"
                      "add eax, ecx\n"
                      "pop rcx\n"
                      "mov [rcx], eax\n";

    run_emitter_test(c_fragment, expected, BLOCK);

}

void test_emit_sub_with_vars() {
    TEST_MSG("sub test");
    char * c_fragment = "{ \n"
                        "int a;\n"
                        "int b;\n"
                        "int c;\n"
                        "a = 43;\n"
                        "b = 1;\n"
                        "c = a - b;\n"
                        "}\n";

    char * expected = "mov eax, 43\n"          // a = 43;
                      "push rax\n"
                      "lea rcx, [rbp-4]\n"
                      "push rcx\n"
                      "pop rcx\n"
                      "pop rax\n"
                      "mov DWORD [rcx], eax\n"
                      "mov eax, 1\n"            // b = 1;
                      "push rax\n"
                      "lea rcx, [rbp-8]\n"
                      "push rcx\n"
                      "pop rcx\n"
                      "pop rax\n"
                      "mov DWORD [rcx], eax\n"  // c = a - b
                      "mov eax, [rbp-4]\n"
                      "push rax\n"
                      "mov eax, [rbp-8]\n"
                      "push rax\n"
                      "pop rcx\n"
                      "pop rax\n"
                      "movsxd rax, eax\n"
                      "movsxd rcx, ecx\n"
                      "sub rax, rcx\n"
                      "push rax\n"
                      "lea rcx, [rbp-12]\n"
                      "push rcx\n"
                      "pop rcx\n"
                      "pop rax\n"
                      "mov DWORD [rcx], eax\n";


    run_emitter_test(c_fragment, expected, BLOCK);

}

void test_emit_sub_assign() {
    TEST_MSG("sub assignment test");
    char * c_fragment = "{ \n"
                        "  int a = 43; \n"
                        "  a -= 1; \n"
                        "}";

    char * expected = "mov eax, 43\n"
                      "push rax\n"
                      "lea rcx, [rbp-4]\n"
                      "push rcx\n"
                      "pop rcx\n"
                      "pop rax\n"
                      "mov DWORD [rcx], eax\n"
                      "lea rcx, [rbp-4]\n"
                      "push rcx\n"
                      "mov DWORD eax, [rcx]\n"
                      "push rax\n"
                      "mov eax, 1\n"
                      "push rax\n"
                      "pop rcx\n"
                      "pop rax\n"
                      "sub eax, ecx\n"
                      "pop rcx\n"
                      "mov [rcx], eax\n";

    run_emitter_test(c_fragment, expected, BLOCK);

}

void test_emit_array_dynamic_indexing_with_assignment() {
    char * c_fragment = "{ \n"
                    "  int a[2];\n"
                    "  int i=1;\n"
                    "  a[i] = 42;\n"
                    "}\n";

    char * expected = "";

    run_emitter_test(c_fragment, expected, BLOCK);

}

void test_emit_array_dynamic_indexing_with_retrieval() {
    char * c_fragment = "{ \n"
                    "  int a[2] = {10, 20};\n"
                    "  int i=1;\n"
                    "  int sum = 0;\n"
                    "  sum = a[i];\n"
                    "}\n";

    char * expected = "";

    run_emitter_test(c_fragment, expected, BLOCK);

}


int main() {
    RUN_TEST(test_emit_basic_expr);
    RUN_TEST(test_emit_literal_with_char_cast);
    RUN_TEST(test_emit_literal_with_int_cast);
    RUN_TEST(test_emit_literal_with_short_cast);
    RUN_TEST(test_emit_int_declaration);
    RUN_TEST(test_emit_add_literals_expr);
    RUN_TEST(test_emit_add_int_var_block);
    RUN_TEST(test_emit_multi_literals_expr);
    RUN_TEST(test_emit_add_assign);
    RUN_TEST(test_emit_sub_with_vars);
    RUN_TEST(test_emit_sub_assign);
    RUN_TEST(test_emit_array_dynamic_indexing_with_assignment);
    RUN_TEST(test_emit_array_dynamic_indexing_with_retrieval);

}

