#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "token.h"
#include "ast.h"
#include "parser_util.h"

ASTNode * create_unary_node(ASTNodeType op, ASTNode * operand) {
    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = op;
    node->unary.operand = operand;
    return node;
}
