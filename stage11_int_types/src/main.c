#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "list_util.h"
#include "ast.h"
#include "token.h"
#include "util.h"

#include "emitter.h"
#include "parser.h"
#include "tokenizer.h"
#include "ast_printer.h"
#include "analyzer.h"
#include "analyzer_context.h"
#include "emitter_context.h"
#include "error.h"
#include "symbol_table.h"

void token_formatted_output(const char * label, const char * text, TokenType tokenType, int num, int line, int col) {
    char left[64];
    char right[64];
    snprintf(left, sizeof(left), "%s: %s", label, text);
    snprintf(right, sizeof(right), "TOKEN_TYPE: %s", token_type_name(tokenType));
    printf("[%6d]  %-25s %-25s%6d%6d\n", num, left, right, line, col);
}

/* main and related */

int main(int argc, char ** argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source file> [-o <output file]\n", argv[0]);
        return 1;
    }

    const char * program_file = NULL;
    const char * output_file = NULL;
    bool output_file_owned = false;

    // parse args
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (!program_file) {
            program_file = argv[i];
        } else {
            error("Unrecognized option: %s", argv[i]);
        }
    }

    if (!program_file) {
        error("No input file given");
    }

    if (!output_file) {
        output_file = change_extension(program_file, ".s");
        output_file_owned = true;
    }

    char * program_text = read_text_file(program_file);
    if (!program_text) {
        error("Could not read file: %s", program_file);
    }

    printf("Compiling\n\n%s\n\n", program_text);

    tokenlist * tokens = tokenize(program_text);

    int i=0;
    // output list
    for (tokenlist_node * node = tokens->head; node != NULL; node = node->next) {
        Token * token = node->value;

        token_formatted_output("TOKEN:", token->text, token->type, ++i, token->line, token->col);
    }

    ASTNode * astNode = parse(tokens);
    
    tokenlist_free(tokens);
    free(tokens);

    printf("\nAST After Parsing\n");
    print_ast(astNode, 0);

    init_global_table();
    AnalyzerContext * ctx = analyzer_context_new();
    analyze(ctx, astNode);
    analyzer_context_free(ctx);

//    populate_symbol_table(astNode, true);

    printf("\nAST After Analyzer\n");
    print_ast(astNode, 0);

    printf("\n");
    printf("--------------------------------------------\n");
    printf("Beginning Code Generation\n");
    printf("--------------------------------------------\n\n\n");


    EmitterContext * emitter_context = create_emitter_context(output_file);
    emit(emitter_context, astNode);

    emitter_finalize(emitter_context);

    // TODO THIS NEEDS TO BE FIXED and OTHER CLEAN AS WELL.
    // cleanup_token_list(&tokenList);

    free_astnode(astNode);
    if (output_file_owned) {
        free((void*)output_file);
    }
    free(program_text);
    exit(0);
}                             