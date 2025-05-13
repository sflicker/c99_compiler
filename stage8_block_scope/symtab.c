#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symtab.h"
#include "util.h"

typedef struct Symbol {
    char* name;
    int offset;
    struct Symbol* next;
} Symbol;

typedef struct Scope {
    Symbol * symbols;
    struct Scope * parent;
} Scope;


// #define MAX_SYMBOLS 128

// typedef struct {
//     Symbol symbols[MAX_SYMBOLS];
//     int count;
//     int next_offset;
// } SymbolTable;

// SymbolTable globalSymTable;

Scope * current_scope = NULL;

int current_offset = 0;
int next_offset = -4;

// void init_symbol_table() {
//     SymbolTable* symbolTable = &globalSymTable;
//     symbolTable->count = 0;
//     symbolTable->next_offset = -4;
// }

void set_current_offset(int offset) {
    current_offset = offset;
    next_offset = offset - 4;
}


void enter_scope() {
    Scope * scope = malloc(sizeof(Scope));
    scope->symbols = NULL;
    scope->parent = current_scope;
    current_scope = scope;
}

void exit_scope() {
    Scope * old = current_scope;
    Symbol * sym = old->symbols;
    while(sym) {
        Symbol * tmp = sym;
        sym = sym->next;
        free(tmp->name);
        free(tmp);
    }
    current_scope = old->parent;
    free(old);
}

void add_symbol(const char * name) {
    // check if symbol currently exists in current scope only
    // error and exit if so.
    Symbol * sym = current_scope->symbols;
    while(sym) {
        if (strcmp(sym->name, name) == 0) {
            fprintf(stderr, "Error: redeclaration of '%s' in same scope\n", name);
            exit(1);
        }
        sym = sym->next;
    }

    // add new symbol
    Symbol * new_symbol = malloc(sizeof(Symbol));
    new_symbol->name = my_strdup(name);
    new_symbol->offset = next_offset;
    next_offset -= 4;

    new_symbol->next = current_scope->symbols;
    current_scope->symbols = new_symbol;

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

int lookup_symbol(const char* name) {
    for (Scope * scope = current_scope; scope != NULL; scope = scope->parent) {
        for (Symbol * sym = scope->symbols; sym != NULL; sym = sym->next) {
            if (strcmp(sym->name, name) == 0) {
                return sym->offset;
            }
        }
    }
    // if not found throw and error and exit
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

int get_symbol_total_space() {

    // TODO COULD MAKE THIS TAKE A PARAMETER THAN SCAN THE TREE BELOW FOR NOW JUST RETURN 64

    return 64;

//     int count=0;
//     for (Scope * scope = current_scope; scope != NULL; scope = scope->parent) {
//         for (Symbol * sym = scope->symbols; sym != NULL; sym = sym->next) {
//             count++;
//         }
//     }
//     return count * 4;
 }