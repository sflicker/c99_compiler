#ifndef AST_LISTS_H
#define AST_LISTS_H

#include "list_util.h"

DEFINE_LINKED_LIST(Token*, tokenlist)
DEFINE_LINKED_LIST(VarDecl*, paramlist)
DEFINE_LINKED_LIST(ASTNode*, arglist)

#endif