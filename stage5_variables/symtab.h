#ifndef _SYMTAB_H_
#define _SYMTAB_H_

typedef struct {
    const char* name;
    int offset;
} Symbol;

#define MAX_SYMBOLS 128

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
    int next_offset;
} SymbolTable;

void init_symbol_table(SymbolTable* symbolTable);
int add_symbol(SymbolTable* table, const char * name);
int lookup_symbol(SymbolTable * table, const char * name);

#endif
