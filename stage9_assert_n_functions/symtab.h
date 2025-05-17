#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "ast.h"

//void init_symbol_table();


int add_symbol(const char * name, struct ASTNode * owner);
int lookup_symbol(const char * name);
int get_symbol_total_space();
void enter_scope();
void exit_scope();
void set_current_offset(int offset);
void reset_storage_size();
int get_node_offset(void * node);
#endif
