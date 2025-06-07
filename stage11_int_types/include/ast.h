#ifndef AST_H
#define AST_H

#include <stdbool.h>

#include "token.h"
#include "ctypes.h"
#include "util.h"
#include "list_util.h"
#include "ast_lists.h"
#include "ast_list.h"

typedef enum {
    AST_RETURN_STMT,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_DO_WHILE_STMT,
    AST_LABELED_STMT,
    AST_CASE_STMT,
    AST_DEFAULT_STMT,
    AST_GOTO_STMT,
    AST_SWITCH_STMT,
    AST_BREAK_STMT,
    AST_CONTINUE_STMT,
    AST_INT_LITERAL,
    AST_FUNCTION_DECL,
    AST_FUNCTION_CALL,
    //AST_PARAM_LIST,
    //AST_ARG_LIST,
    AST_TRANSLATION_UNIT,
    AST_BLOCK,
    AST_EXPRESSION_STMT,
    AST_VAR_DECL,
    AST_VAR_REF,
    AST_ASSIGNMENT,
    AST_COMPOUND_ADD_ASSIGN,
    AST_COMPOUND_SUB_ASSIGN,
    AST_UNARY_POST_INC,
    AST_UNARY_POST_DEC,
    AST_UNARY_PRE_INC,
    AST_UNARY_PRE_DEC,
    AST_UNARY_NEGATE,
    AST_UNARY_NOT,
    AST_UNARY_PLUS,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
    AST_EQUAL,
    AST_NOT_EQUAL,
    AST_LOGICAL_AND,
    AST_LOGICAL_OR,
    AST_LESS_THAN,
    AST_LESS_EQUAL,
    AST_GREATER_THAN,
    AST_GREATER_EQUAL,
    AST_ASSERT_EXTENSION_STATEMENT,
    AST_PRINT_EXTENSION_STATEMENT
} ASTNodeType;

// typedef enum {
//     ADDR_REGISTER,
//     ADDR_STACK,
//     ADDR_UNASSIGNED
// } AddressKind;

// typedef struct Address {
//     AddressKind kind;
//     union {
//         int stack_offset;
//         int reg_index;
//     };
// } Address;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int int_value;

        struct {
            //struct ASTNode ** functions;
            ASTNode_list * functions;
            int count;
        } translation_unit;

        struct {
            char* name;
            CType * return_ctype;
            //struct ASTNode** param_list;
            //paramlist param_list;
            ASTNode_list * param_list;
            //int param_count;
            struct ASTNode* body;
            bool declaration_only;
            int size;
        } function_decl;

        // struct {
        //     struct node_list * node_list;
        // } param_list;

        struct {
            char * name;
            ASTNode_list * arg_list;
            //struct ASTNode ** argument_expression_list;
            //arglist arg_list;
            //int arg_count;
        } function_call;

        struct {
            CType * var_ctype;
            char* name;
            struct ASTNode * init_expr; // NULL if no initializer
//            Address addr;
        } var_decl;

        struct {
            char * name;
//            int offset;
//            Address addr;
        } var_ref;

        struct {
//            struct ASTNode ** statements;
            ASTNode_list * statements;
            int count;
            int capacity;
        } block;

        struct {
            struct ASTNode * expr;
        } return_stmt;

        struct {
            struct ASTNode * lhs;
            struct ASTNode * rhs;
        } binary;

        struct {
            struct ASTNode * operand;
        } unary;


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
            char * name;
//            Address addr;
            struct ASTNode * expr;
        } assignment;

        struct {
            char * label;
            struct ASTNode * stmt;
        } labeled_stmt;

        struct {
            struct ASTNode * constExpression;
            struct ASTNode * stmt;
            char * label;
        } case_stmt;

        struct {
            struct ASTNode * stmt;
            char * label;
        } default_stmt;

        struct {
            char * label;
        } goto_stmt;

        struct {
            struct ASTNode * stmt;
            struct ASTNode * expr;
        } do_while_stmt;

        struct {
            struct ASTNode * expr;
            struct ASTNode * stmt;
        } switch_stmt;
        
    };
} ASTNode;

void free_astnode(ASTNode * node);

#endif