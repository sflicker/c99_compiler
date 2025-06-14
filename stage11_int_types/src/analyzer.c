//
// Created by scott on 6/14/25.
//

#include "ast.h"
#include "analyzer.h"
#include "error.h"
#include "ctypes.h"
#include "symtab.h"

void handle_function_declaration(ASTNode * node) {

    enter_scope();
    CTypePtr_list * typeList = malloc(sizeof(CTypePtr_list));
    CTypePtr_list_init(typeList, free_ctype);
    for (ASTNode_list_node * n = node->function_decl.param_list->head; n != NULL; n = n->next) {
        add_symbol(n->value->var_decl.name, n->value->ctype);
        CTypePtr_list_append(typeList, node->ctype);
    }
    add_function_symbol(node->function_decl.name, node->ctype,
        node->function_decl.param_count, typeList);
    analyze(node->function_decl.body);
    exit_scope();
}

void analyze(ASTNode * node) {
    if (!node) return;

    switch (node->type) {
        case AST_TRANSLATION_UNIT: {
            for (ASTNode_list_node * n = node->translation_unit.functions->head; n; n = n->next) {
                analyze(n->value);
            }
            break;
        }
        case AST_FUNCTION_DECL: {
            handle_function_declaration(node);
            break;
        }
    }
}
