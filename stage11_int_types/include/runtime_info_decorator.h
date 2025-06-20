#ifndef _RUNTIME_INFO_DECORATOR_
#define _RUNTIME_INFO_DECORATOR_

#include "ast.h"
#include "runtime_info.h"

void populate_symbol_table(ASTNode * node);
void init_runtime_info_list();
void add_runtime_info_offset(ASTNode * astNode, int offset);
RuntimeInfo * runtime_info(ASTNode * node);

#endif