//
// Created by scott on 6/15/25.
//

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

const char * current_test = NULL;

void test_analyze_basic_case() {
    init_global_table();
    const char * program = "int main() { return 4+6+12+20; }";

    tokenlist * tokens = tokenize(program);

    ASTNode * actual = parse(tokens);
    print_ast(actual, 0);
    init_global_table();

    AnalyzerContext * context = analyzer_context_new();
    context->current_function_return_type = NULL;
    analyze(context, actual);
    print_ast(actual, 0);
}

void test_analyze_mixed_types() {
    init_global_table();

    const char * program = "int main() {\n"
                       "    int a=1;\n"
                       "    char b=2;\n"
                       "    return a+b;"
                       "}\n";

    TEST_MSG("Input Source Program:");
    TEST_MSG(program);

    tokenlist * tokens = tokenize(program);

    ASTNode * actual = parse(tokens);
    print_ast(actual, 0);
    init_global_table();
    AnalyzerContext * context = analyzer_context_new();
    context->current_function_return_type = &CTYPE_INT_T;
    analyze(context, actual);
    print_ast(actual, 0);
}

void test_analyze_multi_mixed_types() {
    init_global_table();

    const char * program = "int main() {\n"
                           "    int a=1;\n"
                           "    char b=2;\n"
                           "    short c=3;\n"
                           "    return a+b+c;\n"
//                           "    long d=4;\n"
//                           "    return a+b+c+d;\n"
                           "}\n";

    TEST_MSG("Input Source Program:");
    TEST_MSG(program);

    tokenlist * tokens = tokenize(program);

    ASTNode * actual = parse(tokens);
    print_ast(actual, 0);
    init_global_table();
    AnalyzerContext * context = analyzer_context_new();
    context->current_function_return_type = &CTYPE_INT_T;
    analyze(context, actual);
    print_ast(actual, 0);
}

void test_analyze_multi_mixed_types_including_long() {
    init_global_table();

    const char * program = "long main() {\n"
                           "    int a=1;\n"
                           "    char b=2;\n"
                           "    short c=3;\n"
                           "    return a+b+c;\n"
                           "    long d=4;\n"
                           "    return a+b+c+d;\n"
                           "}\n";

    TEST_MSG("Input Source Program:");
    TEST_MSG(program);

    tokenlist * tokens = tokenize(program);

    ASTNode * actual = parse(tokens);
    print_ast(actual, 0);
    init_global_table();
    AnalyzerContext * context = analyzer_context_new();
    context->current_function_return_type = &CTYPE_INT_T;
    analyze(context, actual);
    print_ast(actual, 0);

}

void test_analyze_unary_post_increment() {
    init_global_table();

    const char * program =
        "int main() {\n"
        "    int a=1;\n"
        "    return a++;\n"
        "}\n";

    TEST_MSG("Input Source Program:");
    TEST_MSG(program);

    tokenlist * tokens = tokenize(program);

    ASTNode * actual = parse(tokens);
    print_ast(actual, 0);
    init_global_table();
    AnalyzerContext * context = analyzer_context_new();
    context->current_function_return_type = &CTYPE_INT_T;
    analyze(context, actual);
    print_ast(actual, 0);

}

int main() {
    RUN_TEST(test_analyze_basic_case);
    RUN_TEST(test_analyze_mixed_types);
    RUN_TEST(test_analyze_multi_mixed_types);
    RUN_TEST(test_analyze_multi_mixed_types_including_long);
    RUN_TEST(test_analyze_unary_post_increment);
}