//
// Created by scott on 6/21/25.
//

#ifndef SYMBOL_H
#define SYMBOL_H
#include "ctypes.h"

typedef enum {
    SYMBOL_VAR,
    SYMBOL_FUNC
} SymbolKind;

typedef struct Symbol {
    char * name;
    SymbolKind kind;
    CType * ctype;
    ASTNode * node;

    union {
        struct {
            int offset;
        } var;

        struct {
            ASTNode * func_def_node;
            int num_params;
            Symbol_list * params_symbol_list;
        } func;
    } info;
} Symbol;

void free_symbol(Symbol * symbol);
Symbol * create_symbol(const char * name, SymbolKind kind, CType * t, ASTNode * node);

#endif //SYMBOL_H
