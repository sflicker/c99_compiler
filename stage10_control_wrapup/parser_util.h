#ifndef _PARSER_UTIL_H_
#define _PARSER_UTIL_H_
#include "ast.h"

struct ASTNode * create_unary_node(ASTNodeType op, struct ASTNode * operand);

#endif