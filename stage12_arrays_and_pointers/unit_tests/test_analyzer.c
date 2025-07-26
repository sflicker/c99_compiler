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
#include "c_type.h"

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

void test_variable_references_properly_set() {
    init_global_table();
    char * ref_name = "a";
    CType * ctype = &CTYPE_INT_T;
    ASTNode *varDeclNode = create_var_decl_node(ref_name, ctype, NULL);
    ASTNode *varRefNode = create_var_ref_node(ref_name);

    ASTNode_list * astNodeList = create_node_list();

    ASTNode_list_append(astNodeList, varDeclNode);
    ASTNode_list_append(astNodeList, varRefNode);

    ASTNode * block = create_block_node(astNodeList);

    TEST_ASSERT("Verify reference node now has a NULL ctype", varRefNode->ctype == NULL);

    print_ast(block, 0);
    AnalyzerContext * ctx = analyzer_context_new();
    analyze(ctx, block);

    TEST_ASSERT("Verify reference node now has the proper ctype", varRefNode->ctype == ctype);
}

void test_analyze_array() {
    init_global_table();

    // TEST WILL create a parse tree for the following
    /*
    int main() {
        int a[1];
        a[0] = 30;
        return a[0];    // 30
    }
    */
    // THIS TEST WILL THEN RUN ANALYZE ON THE parse tree

    // finally the test will verify parse tree has been properly updated
    // during the analysis where symbol references etc are set.

    //ASTNode * varDecl = create_var_decl_node("a", make_array_type(&CTYPE_INT_T, 1), NULL);
    

}

int main() {
    RUN_TEST(test_analyze_basic_case);
    RUN_TEST(test_analyze_mixed_types);
    RUN_TEST(test_analyze_multi_mixed_types);
    RUN_TEST(test_analyze_multi_mixed_types_including_long);
    RUN_TEST(test_analyze_unary_post_increment);
    RUN_TEST(test_variable_references_properly_set);
    RUN_TEST(test_analyze_array);
}