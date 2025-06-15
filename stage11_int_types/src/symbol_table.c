#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symbol_table.h"
#include "util.h"
#include "ctype.h"
#include "error.h"


static Scope * current_scope = NULL;
static function_symbol_list * functionSymbolList = NULL;

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
}

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

    FunctionSymbol * new_symbol = malloc(sizeof(FunctionSymbol));
    new_symbol->name = strdup(name);
    new_symbol->return_ctype = returnCType;
    new_symbol->param_count = param_count;
    new_symbol->param_types = param_types;

    function_symbol_list_append(functionSymbolList, new_symbol);

    return new_symbol;
}

Symbol * add_symbol(const char * name, CType * ctype) {
    // check if symbol currently exists in current scope only
    // error and exit if so.

    for (symbollist_node * n = current_scope->symbols->head; n != NULL; n = n->next) {
        if (strcmp(n->value->name, name) == 0) {
            error("Error: redeclaration of '%s' in same scope\n", name);
        }
    }

    // add new symbol
    Symbol * new_symbol = malloc(sizeof(Symbol));
    new_symbol->name = strdup(name);
    new_symbol->ctype = ctype;

    symbollist_append(current_scope->symbols, new_symbol);
    return new_symbol;
}

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
    return NULL;
}

FunctionSymbol * lookup_function_symbol(const char * name) {
    for (function_symbol_list_node * n = functionSymbolList->head; n != NULL; n = n->next) {
        FunctionSymbol * function_symbol = n->value;
        if (strcmp(function_symbol->name, name) == 0) {
            return function_symbol;
        }
    }
    error("Unknown function symbol '%s'\n", name);
    return NULL;
}
