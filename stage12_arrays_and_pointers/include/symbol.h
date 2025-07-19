//
// Created by scott on 6/21/25.
//

#ifndef SYMBOL_H
#define SYMBOL_H
#include "c_type.h"
#define MAX_DIMENSIONS 8

typedef enum {
    SYMBOL_VAR,
    SYMBOL_FUNC,
    SYMBOL_ARRAY
} SymbolKind;

typedef enum {
    STORAGE_LOCAL,
    STORAGE_GLOBAL,
    STORAGE_PARAMETER,
} StorageKind;



typedef struct Symbol {
    char * name;
    SymbolKind kind;
    StorageKind storage;
    CType * ctype;
    ASTNode * node;

    union {
        struct {
            int offset;
            StorageKind storage;
        } var;

        struct {
            ASTNode * func_def_node;
            int num_params;
            Symbol_list * params_symbol_list;
        } func;

        struct {
            int offset;
            int dimensions[MAX_DIMENSIONS];
            int num_dimensions;
            StorageKind storage;
        } array;
    } info;
} Symbol;

void free_symbol(Symbol * symbol);
Symbol * create_symbol(const char * name, SymbolKind kind, CType * t, ASTNode * node);

#endif //SYMBOL_H
