#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "ast.h"
#include "list_util.h"

//void init_symbol_table();

// typedef struct Symbol {
//     char* name;
//     CType * ctype;
//     ASTNode * node;
// //    Address addr;
// //    struct Symbol* next;
// } Symbol;
//
// DEFINE_LINKED_LIST(Symbol*, symbollist);
//
// typedef struct FunctionSymbol {
//     char * name;
//     CType * return_ctype;
//     int param_count;
//     CTypePtr_list * param_types;
// //    struct FunctionSymbol* next;
// } FunctionSymbol;
//
// DEFINE_LINKED_LIST(FunctionSymbol*, function_symbol_list);

// typedef struct Scope {
//     Symbol_list * symbols;
//     struct Scope * parent;
// } Scope;

typedef struct SymbolTable {
    struct SymbolTable * parent;
    Symbol_list * symbols;
} SymbolTable;

SymbolTable * getGlobalScope();
SymbolTable * getCurrentScope();

void init_global_table();
// void symbol_table_push();
// void symbol_table_pop();
Symbol * lookup_symbol(const char * name);
Symbol * lookup_table_symbol(SymbolTable * table, const char * name);


void add_symbol(Symbol * symbol);
void add_global_symbol(Symbol * symbol);

void free_global_table();

//Symbol * add_symbol(const char * name, CType * type, ASTNode * node);
//Symbol * add_symbol_with_offset(const char * name, int offset, Type * type);
//FunctionSymbol * add_function_symbol(const char * name, CType * returnType, int param_count, CTypePtr_list * param_types);


//Symbol * lookup_symbol(const char * name);
//FunctionSymbol * lookup_function_symbol(const char * name);

//int get_symbol_total_space();
void enter_scope();
void exit_scope();
// void free_function_symbol(FunctionSymbol * function_symbol);
// void free_symbol(Symbol * symbol);
// void set_current_offset(int offset);
//  void reset_storage_size();
// int get_node_offset(void * node);
#endif
