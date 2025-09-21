//
// Created by scott on 6/21/25.
//

#include <string.h>

#include "symbol.h"

Symbol * create_symbol(const char * name, SymbolKind kind, CType * ctype, ASTNode * node) {
    Symbol * symbol = (Symbol *) malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->kind = kind;
    symbol->ctype = ctype;
    symbol->node = node;
    return symbol;
}

void free_symbol(Symbol * symbol) {
//    free(symbol->name);
//    free(symbol);
}

Symbol * create_storage_param_symbol(const char * name, ASTNode * node, CType * ctype, int *param_offset) {
    Symbol * symbol = create_symbol(name, SYMBOL_VAR, ctype, node);
    symbol->info.var.offset = *param_offset;
    symbol->info.var.storage = STORAGE_PARAMETER;
    if (is_floating_point_type(ctype)) {
        *param_offset += 16;
    } else {
        *param_offset += 8;
    }
    return symbol;
}