//
// Created by scott on 7/9/25.
//

#include <stdio.h>

#include "test_assert.h"
#include "token.h"
#include "tokenizer.h"
#include "analyzer.h"
#include "parser.h"
#include "parser_context.h"
#include "parser_util.h"
#include "ast_printer.h"
#include "symbol_table.h"
#include "analyzer_context.h"
#include "c_type.h"
#include "emitter.h"
#include "emitter_context.h"

const char * current_test = NULL;

// void strip_comments(char *src, char *dst) {
//     while (*src) {
//         if (*src == ';') {
//             // skip to end of line
//             while (*src && *src != '\n') src++;
//         }
//         if (*src) {
//             *dst++ = *src++;
//         }
//     }
//     *dst = '\0';
// }

void strip_comments(char *src, char *dst) {
    while (*src) {
        // Skip leading whitespace to peek at first non-whitespace
        char *line_start = src;
        while (*src == ' ' || *src == '\t') src++;

        if (*src == ';') {
            // full line comment: skip to end of line
            while (*src && *src != '\n') src++;
            if (*src == '\n') src++;  // skip newline too
            continue;
        }

        // copy line while stripping inline comments and trailing spaces before ;
        while (*line_start && *line_start != '\n') {
            if (*line_start == ';') {
                // backtrack over spaces in dst
                while (dst > src && (dst[-1] == ' ' || dst[-1] == '\t'))
                    dst--;
                // skip to end of line
                while (*line_start && *line_start != '\n') line_start++;
                break;
            } else {
                *dst++ = *line_start++;
            }
        }

        // copy newline if present
        if (*line_start == '\n') {
            *dst++ = *line_start++;
        }

        src = line_start;
    }

    *dst = '\0';
}

void run_emitter_test(char * c_fragment, char * expected) {
    tokenlist * tokens = tokenize(c_fragment);
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node = parse_expression(ctx);
    printf("\nAST After Parsing\n");
    print_ast(node, 0);
    free_parser_context(ctx);
    tokenlist_free(tokens);

    init_global_table();
    AnalyzerContext * analyzer_context = analyzer_context_new();
    analyze(analyzer_context, node);
    analyzer_context_free(analyzer_context);

    printf("AST After Analyzer\n");
    print_ast(node, 0);

    char * buffer = NULL;
    size_t buffer_size = 0;

    FILE * memf = open_memstream(&buffer, &buffer_size);

    EmitterContext * emitter_context = create_emitter_context_from_fp(memf);
    emit_tree_node(emitter_context, node);
    emitter_finalize(emitter_context);

    printf("Generated code:\n%s\n", buffer);
    char * normalizedOutput = malloc(strlen(buffer) + 1);

    strip_comments(buffer, normalizedOutput);

    TEST_ASSERT("Verifying generated code is correct", strcmp(expected, normalizedOutput) == 0);

    free(normalizedOutput);
}


void test_emit_basic_expr() {
    TEST_MSG("Basic expression test");
    char * c_fragment = "42";
    char * expected =
        "mov eax, 42\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected);
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

    run_emitter_test(c_fragment, expected);

}

void test_emit_literal_with_int_cast() {
    TEST_MSG("test int cast of literal");
    char * c_fragment = "(int)42";
    char * expected =
        "mov eax, 42\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected);
}

void test_emit_literal_with_char_cast() {
    char * c_fragment = "(char)42";

    TEST_MSG("test char cast of literal");

    char * expected =
        "mov eax, 42\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected);
}

void test_emit_literal_with_short_cast() {
    TEST_MSG("test short cast of literal");
    char * c_fragment = "(short)42";
    char * expected =
        "mov eax, 42\n"
        "push rax\n";

    run_emitter_test(c_fragment, expected);
}

int main() {
    RUN_TEST(test_emit_basic_expr);
    RUN_TEST(test_emit_add_literals_expr);
    RUN_TEST(test_emit_literal_with_int_cast);
    RUN_TEST(test_emit_literal_with_char_cast);
    RUN_TEST(test_emit_literal_with_short_cast);
}

