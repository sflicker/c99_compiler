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

void strip_comments(char *src, char *dst) {
    while (*src) {
        if (*src == ';') {
            // skip to end of line
            while (*src && *src != '\n') src++;
        }
        if (*src) {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

void test_emit_basic_expr() {
    tokenlist * tokens = tokenize("42");
    ParserContext * ctx = create_parser_context(tokens);

    ASTNode * node = parse_primary(ctx);
    print_ast(node, 0);
    free_parser_context(ctx);
    tokenlist_free(tokens);

    char * buffer = NULL;
    size_t buffer_size = 0;

    FILE * memf = open_memstream(&buffer, &buffer_size);

    EmitterContext * emitter_context = create_emitter_context_from_fp(memf);
    emit_tree_node(emitter_context, node);

    printf("Generated code:\n%s\n", buffer);


}

int main() {
    RUN_TEST(test_emit_basic_expr);
}

