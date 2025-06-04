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
#include "runtime_info_decorator.h"
#include "ast_printer.h"
#include "tokenizer_context.h"

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
        fprintf(stderr, "Usage: %s <source file>\n", argv[0]);
        return 1;
    }
    const char * program_file = argv[1];
    char * output_file = change_extension(program_file, ".s");

    char * program_text = read_text_file(program_file);
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

//    populate_symbol_table(astNode, true);
    print_ast(astNode, 0);

//    emit(astNode, output_file);

    // TODO THIS NEEDS TO BE FIXED and OTHER CLEAN AS WELL.
    // cleanup_token_list(&tokenList);

    free_astnode(astNode);
    free(output_file);
    free(program_text);
    exit(0);
}                             