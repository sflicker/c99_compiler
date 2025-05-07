#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum {
    AST_RETURN_STMT,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_INT_LITERAL,
    AST_FUNCTION,
    AST_PROGRAM,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_BLOCK,
    AST_EXPRESSION_STMT,
    AST_VAR_DECL,
    AST_VAR_EXPR,
    AST_ASSIGNMENT,
    AST_COMPOUND_ADD_ASSIGN,
    AST_COMPOUND_SUB_ASSIGN
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
            int capacity;
        } block;

        struct {
            struct ASTNode * cond;
            struct ASTNode * then_statement;
            struct ASTNode * else_statement;
        } if_stmt;

        struct {
            struct ASTNode * cond;
            struct ASTNode * body;
        } while_stmt;

        struct {
            struct ASTNode * init_expr;
            struct ASTNode * cond_expr;
            struct ASTNode * update_expr;
            struct ASTNode * body;
        } for_stmt;

        struct {
            struct ASTNode * expr;
        } expr_stmt;

        struct {
            char* name;
            struct ASTNode * init_expr; // NULL if no initializer
        } declaration;

        struct {
            char * name;
            struct ASTNode * expr;
        } assignment;

        struct {
            char * name;
        } var_expression;

    };
} ASTNode;

#endif