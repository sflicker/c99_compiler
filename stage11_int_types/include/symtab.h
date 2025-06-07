#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "ast.h"

//void init_symbol_table();

typedef struct Symbol {
    char* name;
    CType * ctype;
//    Address addr;
    struct Symbol* next;
} Symbol;

typedef struct FunctionSymbol {
    char * name;
    CType * return_ctype;
    int param_count;
    CTypePtr_list * param_types;
    struct FunctionSymbol* next;
} FunctionSymbol;

typedef struct Scope {
    Symbol * symbols;
    struct Scope * parent;
} Scope;

Symbol * add_symbol(const char * name, CType * type);
//Symbol * add_symbol_with_offset(const char * name, int offset, Type * type);
void add_function_symbol(const char * name, CType * returnType, int param_count, CTypePtr_list * param_types);
Symbol * lookup_symbol(const char * name);
int get_symbol_total_space();
void enter_scope();
void exit_scope();
void set_current_offset(int offset);
void reset_storage_size();
int get_node_offset(void * node);
#endif
