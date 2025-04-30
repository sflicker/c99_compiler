#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum {
    AST_RETURN_STMT,
    AST_INT_LITERAL,
    AST_FUNCTION,
    AST_PROGRAM,
    AST_BINARY_OP
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int int_value;

        struct {
            struct ASTNode * expr;
        } return_stmt;

        struct {
            const char* name;
            struct ASTNode* body;
        } function;

        struct {
            struct ASTNode * function;
        } program;

        struct {
            struct ASTNode * lhs;
            TokenType op;
            struct ASTNode * rhs;
        } binary_op;

    };
} ASTNode;

#endif