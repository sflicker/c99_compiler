#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symbol_table.h"
#include "util.h"
#include "ctype.h"
#include "error.h"


// #define MAX_SYMBOLS 128

// typedef struct {
//     Symbol symbols[MAX_SYMBOLS];
//     int count;
//     int next_offset;
// } SymbolTable;

// SymbolTable globalSymTable;

static Scope * current_scope = NULL;
static function_symbol_list * functionSymbolList = NULL;

// int current_offset = 0;
// int next_offset = -4;
//
// int storage_size = 0;

//struct node_list * node_to_offset_list = NULL;

void free_function_symbol(FunctionSymbol * function_symbol) {
    free(function_symbol->name);
    CTypePtr_list_free(function_symbol->param_types);
    free(function_symbol);
}

void free_symbol(Symbol * symbol) {
    free(symbol->name);
    free(symbol);
}

void init_symbol_table() {
    functionSymbolList = (function_symbol_list *) malloc(sizeof(function_symbol_list));
    function_symbol_list_init(functionSymbolList, free_function_symbol);
//     SymbolTable* symbolTable = &globalSymTable;
//     symbolTable->count = 0;
//     symbolTable->next_offset = -4;
}

// void set_current_offset(int offset) {
//     current_offset = offset;
//     next_offset = offset - 4;
// }

// void reset_storage_size() {
//     storage_size = 0;
// }

void enter_scope() {
    Scope * scope = malloc(sizeof(Scope));
    scope->symbols = malloc(sizeof(symbollist));
    symbollist_init(scope->symbols, free_symbol);
    scope->parent = current_scope;
    current_scope = scope;
}

void exit_scope() {
    Scope * old = current_scope;
    symbollist_free(current_scope->symbols);

    // Symbol * sym = old->symbols;
    // while(sym) {
    //     Symbol * tmp = sym;
    //     sym = sym->next;
    //     free(tmp->name);
    //     free(tmp);
    // }
    current_scope = old->parent;
    free(old);
}

FunctionSymbol * add_function_symbol(const char * name, CType * returnCType, int param_count, CTypePtr_list * param_types) {

    for (function_symbol_list_node * n = functionSymbolList->head; n != NULL; n = n->next) {
        FunctionSymbol * function_symbol = n->value;
        if (strcmp(function_symbol->name, name) == 0) {
            error("Error: redeclaration of '%s' in same scope\n", name);
        }
    }

    // FunctionSymbol * sym = functionSymbolList;
    // while(sym) {
    //     if (strcmp(sym->name, name) == 0) {
    //         error("Error: redeclaration of '%s' in same scope\n", name);
    //     }
    //     sym = sym->next;
    // }

    FunctionSymbol * new_symbol = malloc(sizeof(FunctionSymbol));
    new_symbol->name = strdup(name);
    new_symbol->return_ctype = returnCType;
    new_symbol->param_count = param_count;
    new_symbol->param_types = param_types;

    function_symbol_list_append(functionSymbolList, new_symbol);

    // new_symbol->next = functionSymbolList;
    // functionSymbolList = new_symbol;

    return new_symbol;
}

// int add_symbol_with_offset(const char * name, int offset, Type * type) {
//     // check if symbol currently exists in current scope only
//     // error and exit if so.
//     Symbol * sym = current_scope->symbols;
//     while(sym) {
//         if (strcmp(sym->name, name) == 0) {
//             error("Error: redeclaration of '%s' in same scope\n", name);
//         }
//         sym = sym->next;
//     }

//     // add new symbol
//     Symbol * new_symbol = malloc(sizeof(Symbol));
//     new_symbol->name = strdup(name);
//     new_symbol->type = type;
// //    new_symbol->offset = offset;

//     new_symbol->next = current_scope->symbols;
//     current_scope->symbols = new_symbol;
    
//     // only update for negative offsets which are for local variables
//     if (offset < 0) {
//         storage_size += type->size;
//     }
//     return new_symbol->offset;

// }

Symbol * add_symbol(const char * name, CType * ctype) {
    // check if symbol currently exists in current scope only
    // error and exit if so.

    for (symbollist_node * n = current_scope->symbols->head; n != NULL; n = n->next) {
        if (strcmp(n->value->name, name) == 0) {
            error("Error: redeclaration of '%s' in same scope\n", name);
        }
    }

    // Symbol * sym = current_scope->symbols;
    // while(sym) {
    //     if (strcmp(sym->name, name) == 0) {
    //         fprintf(stderr, "Error: redeclaration of '%s' in same scope\n", name);
    //         exit(1);
    //     }
    //     sym = sym->next;
    // }

    // add new symbol
    Symbol * new_symbol = malloc(sizeof(Symbol));
    new_symbol->name = strdup(name);
  //  new_symbol->offset = next_offset;
    new_symbol->ctype = ctype;

    symbollist_append(current_scope->symbols, new_symbol);
    // next_offset -= ctype->size;
    //
    // new_symbol->next = current_scope->symbols;
    // current_scope->symbols = new_symbol;
    // storage_size += ctype->size;
    return new_symbol;
}

//int add_symbol(const char * name) {
//    SymbolTable * table = &globalSymTable;
    // for (int i=0;i<table->count;i++) {
    //     if (strcmp(table->symbols[i].name, name) == 0) {
    //         return table->symbols[i].offset;   // already defined
    //     }
    // }

    // int offset = table->next_offset;
    // table->symbols[table->count].name = my_strdup(name);
    // table->symbols[table->count].offset = offset;
    // table->count++;
    // table->next_offset -= 4;
    // return offset;
//}

// int lookup_symbol(const char * name) {
//     SymbolTable * table = &globalSymTable;
//     for (int i=0;i<table->count;i++) {
//         if (strcmp(table->symbols[i].name, name) == 0) {
//             return table->symbols[i].offset;
//         }
//     }
//     fprintf(stderr, "Undefined variable: %s\n", name);
//     exit(1);
// }

Symbol * lookup_symbol(const char* name) {
    for (Scope * scope = current_scope; scope != NULL; scope = scope->parent) {
        for (symbollist_node * n = scope->symbols->head; n != NULL; n = n->next) {
            if (strcmp(n->value->name, name) == 0) {
                return n->value;
            }
        }
    }
    // if not found throw and error and exit
    error("Unknown symbol '%s'\n", name);
    // fprintf(stderr, "Undefined variable: %s\n", name);
    // exit(1);
    return NULL;
}

// int get_symbol_total_space() {
//
//     int size = storage_size;
//     // round up to a 16 byte
//     if (size > 0) {
//         int remainder = storage_size % 16;
//         if (remainder > 0) {
//             size = storage_size + (16 - remainder);
//         }
//     }
//
//     return size;
//
//     // TODO COULD MAKE THIS TAKE A PARAMETER THAN SCAN THE TREE BELOW FOR NOW JUST RETURN 64
//
//     //return 64;
//
// //     int count=0;
// //     for (Scope * scope = current_scope; scope != NULL; scope = scope->parent) {
// //         for (Symbol * sym = scope->symbols; sym != NULL; sym = sym->next) {
// //             count++;
// //         }
// //     }
// //     return count * 4;
//  }