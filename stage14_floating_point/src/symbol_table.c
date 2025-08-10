#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symbol.h"
#include "symbol_table.h"
#include "util.h"
#include "ctype.h"
#include "error.h"

SymbolTable * global_scope = NULL;
SymbolTable * current_scope = NULL;

SymbolTable * getGlobalScope() {
    return global_scope;
}

SymbolTable * getCurrentScope() {
    return current_scope;
}

void init_global_table() {
    global_scope = (SymbolTable *) malloc(sizeof(SymbolTable));
    global_scope->symbols = malloc(sizeof(Symbol_list));
    Symbol_list_init(global_scope->symbols, free_symbol);
    global_scope->parent = NULL;

    current_scope = global_scope;
    current_scope->parent = NULL;
}

void free_symbol_table(SymbolTable * table) {
    SymbolTable * old = table;
    table = table->parent;
    free(old);
}

void enter_scope() {
    SymbolTable * new_scope = (SymbolTable *) malloc(sizeof(SymbolTable));
    new_scope->symbols = malloc(sizeof(Symbol_list));
    Symbol_list_init(new_scope->symbols, free_symbol);
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

void exit_scope() {
    if (current_scope == NULL) {
        error("tried to pop symbol table but no current_scope");
    }
    SymbolTable * old_scope = current_scope;
    current_scope = old_scope->parent;
    Symbol_list_free(old_scope->symbols);
    free(old_scope);
}

Symbol * lookup_symbol(const char * name) {

    SymbolTable * scope = current_scope;

    while (scope) {
        for (Symbol_list_node * n = scope->symbols->head; n != NULL; n = n->next) {
            if (strcmp(n->value->name , name) == 0) {
                return n->value;
            }
        }
        scope = scope->parent;
    }
    return NULL;
}

Symbol * lookup_table_symbol(SymbolTable * table, const char * name) {
    for (Symbol_list_node * n = table->symbols->head; n != NULL; n = n->next) {
        if (strcmp(n->value->name , name) == 0) {
            return n->value;
        }
    }
    return NULL;
}

void add_symbol(Symbol * symbol) {
    Symbol_list_append(current_scope->symbols, symbol);
}

void add_global_symbol(Symbol * symbol) {
    Symbol_list_append(global_scope->symbols, symbol);
}
