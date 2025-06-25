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

}
