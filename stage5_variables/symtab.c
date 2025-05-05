#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symtab.h"
#include "util.h"

#define MAX_SYMBOLS 128

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
    int next_offset;
} SymbolTable;

SymbolTable globalSymTable;

void init_symbol_table() {
    SymbolTable* symbolTable = &globalSymTable;
    symbolTable->count = 0;
    symbolTable->next_offset = -4;
}

int add_symbol(const char * name) {
    SymbolTable * table = &globalSymTable;
    for (int i=0;i<table->count;i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return table->symbols[i].offset;   // already defined
        }
    }

    int offset = table->next_offset;
    table->symbols[table->count].name = my_strdup(name);
    table->symbols[table->count].offset = offset;
    table->count++;
    table->next_offset -= 4;
    return offset;
}

int lookup_symbol(const char * name) {
    SymbolTable * table = &globalSymTable;
    for (int i=0;i<table->count;i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return table->symbols[i].offset;
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

int get_symbol_total_space() {
    SymbolTable * table = &globalSymTable;
    return table->count * 4;
}