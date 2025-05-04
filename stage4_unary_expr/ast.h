#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum {
    AST_RETURN_STMT,
    AST_INT_LITERAL,
    AST_FUNCTION,
    AST_PROGRAM,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_BLOCK
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

        struct {
            struct ASTNode * expr;
            TokenType op;
        } unary_op;

        struct {
            struct ASTNode ** statements;
            int count;
        } block;

        struct {
            struct ASTNode * cond;
            struct ASTNode * then_branch;
            struct ASTNode * else_branch;
        } if_stmt;

    };
} ASTNode;

#endif