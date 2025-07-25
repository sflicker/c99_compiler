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
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_EQ,
    BINOP_NE,
    BINOP_GT,
    BINOP_GE,
    BINOP_LT,
    BINOP_LE,
    BINOP_LOGICAL_OR,
    BINOP_LOGICAL_AND,
    BINOP_UNASSIGNED_OP,
    BINOP_ASSIGNMENT,
    BINOP_COMPOUND_ADD_ASSIGN,
    BINOP_COMPOUND_SUB_ASSIGN
} BinaryOperator;

typedef struct {
    ASTNode * lhs;
    ASTNode * rhs;
    BinaryOperator op;
} BinaryExpr;

typedef enum {
    UNARY_NEGATE,         // -a
    UNARY_PLUS,           // +a    usually this is just a noop
    UNARY_NOT,            // !a
    UNARY_PRE_INC,        // ++a
    UNARY_PRE_DEC,        // --a
    UNARY_POST_INC,       // a++
    UNARY_POST_DEC,       // a--
    UNARY_UNASSIGNED_OP   // op is not assigned. 
} UnaryOperator;

typedef struct {
    ASTNode * operand;
    UnaryOperator op;
} UnaryExpr;

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
    AST_TRANSLATION_UNIT,
    AST_BLOCK,
    AST_EXPRESSION_STMT,
    AST_VAR_DECL,
    AST_VAR_REF,
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_ASSERT_EXTENSION_STATEMENT,
    AST_PRINT_EXTENSION_STATEMENT
} ASTNodeType;

typedef struct ASTNode {
    Symbol * symbol;
    ASTNodeType type;
    CType * ctype;
    union {
        int int_value;
        BinaryExpr binary;
        UnaryExpr unary;
        struct {
            ASTNode_list * functions;
            int count;
        } translation_unit;

        struct {
            char* name;
            ASTNode_list * param_list;
            int param_count;
            ASTNode* body;
            bool declaration_only;
            int size;
        } function_decl;

        struct {
            char * name;
            ASTNode_list * arg_list;
            int arg_count;
        } function_call;

        struct {
            char* name;
            struct ASTNode * init_expr; // NULL if no initializer
            bool is_param;
        } var_decl;

        struct {
            char * name;
        } var_ref;

        struct {
            ASTNode_list * statements;
            bool introduce_scope;
            int count;
        } block;

        struct {
            struct ASTNode * expr;
        } return_stmt;

        struct {
            ASTNode * cond;
            ASTNode * then_stmt;
            ASTNode * else_stmt;
        } if_stmt;

        struct {
            ASTNode * cond;
            ASTNode * body;
        } while_stmt;

        struct {
            ASTNode * init_expr;
            ASTNode * cond_expr;
            ASTNode * update_expr;
            ASTNode * body;
        } for_stmt;

        struct {
            ASTNode * expr;
        } expr_stmt;

        struct {
            char * label;
            ASTNode * stmt;
        } labeled_stmt;

        struct {
            ASTNode * constExpression;
            ASTNode * stmt;
            const char * label;            // maybe generated by compiler
        } case_stmt;

        struct {
            ASTNode * stmt;
            const char * label;             // maybe gneerated by compiler
        } default_stmt;

        struct {
            char * label;
        } goto_stmt;

        struct {
            ASTNode * body;
            ASTNode * expr;
        } do_while_stmt;

        struct {
            ASTNode * expr;
            ASTNode * stmt;
        } switch_stmt;
        
    };
} ASTNode;

void free_astnode(ASTNode * node);

BinaryOperator get_binary_operator_from_tok(Token * tok);
const char * get_binary_op_name(BinaryOperator op);
const char * get_unary_op_name(UnaryOperator op);
bool ast_equal(ASTNode * a, ASTNode * b);

#endif
