#ifndef _SYMTAB_H_
#define _SYMTAB_H_


//void init_symbol_table();


void add_symbol(const char * name);
int lookup_symbol(const char * name);
int get_symbol_total_space();
void enter_scope();
void exit_scope();
void set_current_offset(int offset);
#endif
