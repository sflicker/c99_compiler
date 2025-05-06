#ifndef _SYMTAB_H_
#define _SYMTAB_H_

typedef struct {
    const char* name;
    int offset;
} Symbol;



void init_symbol_table();
int add_symbol(const char * name);
int lookup_symbol(const char * name);
int get_symbol_total_space();

#endif
