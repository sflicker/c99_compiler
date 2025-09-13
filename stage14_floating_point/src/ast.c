#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"
#include "ast.h"

int ast_id = 0;

ASTNode * create_ast() {
    ASTNode * ast_node = calloc(1, sizeof(ASTNode));
    ast_node->id = ast_id++;
    return ast_node;
}

void free_ast(ASTNode * node) {
    if (!node) return;
    switch(node->type) {
        case AST_TRANSLATION_UNIT:
            ASTNode_list_free(node->translation_unit.functions);
            free(node->translation_unit.functions);
            break;

        case AST_VAR_DECL:
            free(node->var_decl.name);
            free_ast(node->var_decl.init_expr);
        break;

        case AST_FUNCTION_DECL:
            free(node->function_decl.name);
            if (node->function_decl.param_list != NULL) {
                ASTNode_list_free(node->function_decl.param_list);
                free(node->function_decl.param_list);
            }
//            free_ast(node->function_decl.body);
            break;

        case AST_FUNCTION_DEF:
            free(node->function_def.name);
            if (node->function_def.param_list != NULL) {
                ASTNode_list_free(node->function_def.param_list);
                free(node->function_def.param_list);
            }
            free_ast(node->function_def.body);
            break;

        case AST_FUNCTION_CALL_EXPR:
            ASTNode_list_free(node->function_call.arg_list);
            free(node->function_call.arg_list);
            break;

        case AST_RETURN_STMT:
            free_ast(node->return_stmt.expr);
            break;

        case AST_BLOCK_STMT:
            ASTNode_list_free(node->block.statements);
            free(node->block.statements);
            break;

        case AST_IF_STMT:
            free_ast(node->if_stmt.cond);
            free_ast(node->if_stmt.then_stmt);
            free_ast(node->if_stmt.else_stmt);
            break;

        case AST_WHILE_STMT:
            free_ast(node->while_stmt.cond);
            free_ast(node->while_stmt.body);
            break;

        case AST_FOR_STMT:
            free_ast(node->for_stmt.init_expr);
            free_ast(node->for_stmt.cond_expr);
            free_ast(node->for_stmt.update_expr);
            free_ast(node->for_stmt.body);
            break;

        case AST_ASSERT_EXTENSION_STATEMENT:
        case AST_PRINT_EXTENSION_STATEMENT:
        case AST_EXPRESSION_STMT:
            free_ast(node->expr_stmt.expr);
            break;

        case AST_VAR_REF_EXPR:
            free(node->var_ref.name);
            break;
        case AST_ARRAY_ACCESS:
            free_ast(node->array_access.base);
            free_ast(node->array_access.index);
            break;
        case AST_LABELED_STMT:
            free(node->labeled_stmt.label);
            free_ast(node->labeled_stmt.stmt);
            break;

        case AST_SWITCH_STMT:
            free_ast(node->switch_stmt.expr);
            free_ast(node->switch_stmt.stmt);
            break;

        case AST_CASE_STMT:
            free_ast(node->case_stmt.constExpression);
            free_ast(node->case_stmt.stmt);
            break;

        case AST_DEFAULT_STMT:
            free_ast(node->default_stmt.stmt);
            break;

        case AST_GOTO_STMT:
            free(node->goto_stmt.label);
            break;

        case AST_BINARY_EXPR:
            free_ast(node->binary.lhs);
            free_ast(node->binary.rhs);
            break;

        case AST_UNARY_EXPR:
            free_ast(node->unary.operand);
            break;

        case AST_DO_WHILE_STMT:
            free_ast(node->do_while_stmt.expr);
            free_ast(node->do_while_stmt.body);
            break;

        case AST_CAST_EXPR:
            free_ast(node->cast_expr.expr);
            break;

        case AST_CONTINUE_STMT:
        case AST_BREAK_STMT:
        case AST_INT_LITERAL:
        case AST_INITIALIZER_LIST:
        case AST_FLOAT_LITERAL:
        case AST_DOUBLE_LITERAL:
            // DO NOTHING
            break;

        case AST_STRING_LITERAL:
            free(node->string_literal.value);
            break;

        case AST_DECLARATION_STMT:
            ASTNode_list_free(node->declaration.init_declarator_list);
            break;

        default: 
            error("Invalid AST Node Type: %s\n", get_ast_node_name(node));
            break;
    }
    free(node);
}

BinaryOperator get_binary_operator_from_tok(Token * tok) {
    switch (tok->type) {
        case TOKEN_PLUS: return BINOP_ADD; break;
        case TOKEN_MINUS: return BINOP_SUB; break;
        case TOKEN_STAR: return BINOP_MUL; break;
        case TOKEN_DIV: return BINOP_DIV; break;
        case TOKEN_PERCENT: return BINOP_MOD; break;
        case TOKEN_GT: return BINOP_GT; break;
        case TOKEN_GE: return BINOP_GE; break;
        case TOKEN_LT: return BINOP_LT; break;
        case TOKEN_LE: return BINOP_LE; break;
        default:
            error("Unknown binary operator '%s'\n", tok->text);
            return BINOP_UNASSIGNED_OP;\
    }
}

const char * get_binary_op_name(BinaryOperator op) {
    switch (op) {
        case BINOP_ADD: return "ADD"; break;
        case BINOP_SUB: return "SUB"; break;
        case BINOP_MUL: return "MUL"; break;
        case BINOP_DIV: return "DIV"; break;
        case BINOP_MOD: return "MOD"; break;
        case BINOP_EQ: return "EQ"; break;
        case BINOP_NE: return "NE"; break;
        case BINOP_GT: return "GT"; break;
        case BINOP_GE: return "GE"; break;
        case BINOP_LT: return "LT"; break;
        case BINOP_LE: return "LE"; break;
        case BINOP_LOGICAL_OR: return "OR"; break;
        case BINOP_LOGICAL_AND: return "AND"; break;
        case BINOP_ASSIGNMENT: return "ASN"; break;
        case BINOP_COMPOUND_ADD_ASSIGN: return "ADDASN"; break;
        case BINOP_COMPOUND_SUB_ASSIGN: return "SUBASN"; break;
        case BINOP_UNASSIGNED_OP: return "UNASSIGNED"; break;
    }
    return "UNASSIGNED";
}

const char * get_unary_op_name(UnaryOperator op) {
    switch(op) {
        case UNARY_NEGATE: return "NEGATE"; break;
        case UNARY_PLUS: return "PLUS"; break;
        case UNARY_NOT: return "NOT"; break;
        case UNARY_PRE_INC: return "PRE_INC"; break;
        case UNARY_PRE_DEC: return "PRE_DEC"; break;
        case UNARY_POST_INC: return "POST_INC"; break;
        case UNARY_POST_DEC: return "POST_DEC"; break;
        case UNARY_ADDRESS: return "ADDRESS"; break;
        case UNARY_DEREF: return "DEREF"; break;
        case UNARY_UNASSIGNED_OP: return "UNASSIGNED"; break;
    }
    return "UNKNOWN";
}

const char * get_ast_node_name(ASTNode * node) {
    switch (node->type) {
        case AST_RETURN_STMT: return "ReturnStmt";
        case AST_IF_STMT: return "IfStmt";
        case AST_WHILE_STMT: return "WhileStmt";
        case AST_FOR_STMT: return "ForStmt";
        case AST_BREAK_STMT: return "BreakStmt";
        case AST_CONTINUE_STMT: return "ContinueStmt";
        case AST_GOTO_STMT: return "GotoStmt";
        case AST_SWITCH_STMT: return "SwitchStmt";
        case AST_CASE_STMT: return "CaseStmt";
        case AST_DEFAULT_STMT: return "DefaultStmt";
        case AST_FUNCTION_DECL: return "FunctionDecl";
        case AST_FUNCTION_DEF:  return "FunctionDef";
        case AST_FUNCTION_CALL_EXPR: return "FunctionCallExpr";
        case AST_VAR_DECL: return "VarDecl";
        case AST_DECLARATION_STMT: return "DeclStmt";
        case AST_VAR_REF_EXPR: return "VarRefExpr";
        case AST_TRANSLATION_UNIT: return "TranslationUnit";
        case AST_BLOCK_STMT: return "BlockStmt";
        case AST_EXPRESSION_STMT: return "ExpressionStmt";
        case AST_BINARY_EXPR: return "BinaryExpr";
        case AST_UNARY_EXPR: return "UnaryExpr";
        case AST_CAST_EXPR: return "CastExpr";
        case AST_DO_WHILE_STMT: return "DoWhileStmt";
        case AST_LABELED_STMT: return "LabelledStmt";
        case AST_INT_LITERAL: return "IntLiteral";
        case AST_FLOAT_LITERAL: return "FloatLiteral";
        case AST_DOUBLE_LITERAL: return "DoubleLiteral";
        case AST_ARRAY_ACCESS: return "ArrayAccess";
        case AST_INITIALIZER_LIST: return "InitializerList";
        case AST_STRING_LITERAL: return "StringLiteral";
        case AST_ASSERT_EXTENSION_STATEMENT: return "AssertExtensionStatement";
        case AST_PRINT_EXTENSION_STATEMENT: return "PrintExtensionStatement";
    }
    error("Invalid AST Node Type: %d", node->type);

    return NULL;
}

bool binop_equal(BinaryOperator a, BinaryOperator b) {
    if (a != b) {
        fprintf(stderr, "binop do not match %s, %s",
            get_binary_op_name(a), get_binary_op_name(b));
        return false;
    }
    return true;
}

bool ast_equal(ASTNode * a, ASTNode * b) {
    if (!a || !b) return a == b;

    if (a->type != b->type) {
        fprintf(stderr, "ast node types do not match %s, %s\n", get_ast_node_name(a), get_ast_node_name(b));
        return false;
    }
    if (!ctype_equals(a->ctype, b->ctype)) {
        char buf_lhs[128], buf_rhs[128];
        ctype_to_cdecl(a->ctype, buf_lhs, sizeof(buf_lhs));
        ctype_to_cdecl(b->ctype, buf_rhs, sizeof(buf_rhs));
        fprintf(stderr, "ast node ctypes do not match %s, %s\n",
            buf_lhs, buf_rhs);
        return false;
    }
    switch (a->type) {
        case AST_DECLARATION_STMT: {
            if (!ctype_equals(a->declaration.declaration_type, b->declaration.declaration_type))
                return false;
            for (int i=0;i<a->declaration.init_declarator_list->count;i++) {
                ASTNode * a_init_decl = ASTNode_list_get(a->declaration.init_declarator_list, i);
                ASTNode * b_init_decl = ASTNode_list_get(b->declaration.init_declarator_list, i);
                if (!ast_equal(a_init_decl, b_init_decl)) return false;
            }
            return true;
            break;
        }
        case AST_VAR_DECL:
            if (!strcmp(a->var_decl.name, b->var_decl.name) == 0) {
                fprintf(stderr, "ast variable names do not match - %s, %s\n",
                    a->var_decl.name, a->var_decl.name);
                return false;
            }
            return true;
            break;
        case AST_VAR_REF_EXPR:
            if (!strcmp(a->var_ref.name, b->var_ref.name) == 0) {
                fprintf(stderr, "ast variable names do not match - %s, %s\n",
                    a->var_ref.name, b->var_ref.name);
                return false;
            }
            return true;
            break;
        case AST_INT_LITERAL:
            return a->int_value == b->int_value;
            break;
        case AST_BINARY_EXPR:
            return (binop_equal(a->binary.op, b->binary.op)) &&
                ast_equal(a->binary.lhs, b->binary.lhs ) &&
                ast_equal(a->binary.rhs, b->binary.rhs);
        case AST_UNARY_EXPR:
            return (a->unary.op == b->unary.op) &&
                ast_equal(a->unary.operand, b->unary.operand);
        case AST_FUNCTION_CALL_EXPR:
            return (strcmp(a->function_call.name, b->function_call.name) == 0 &&
                ctype_lists_equal(
                    astNodeListToTypeList(a->function_call.arg_list),
                    astNodeListToTypeList(b->function_call.arg_list)
                )
            );
        case AST_EXPRESSION_STMT:
        case AST_RETURN_STMT:
            return ast_equal(a->return_stmt.expr, b->return_stmt.expr);
            break;
        case AST_CAST_EXPR:
            return ctype_equals(a->cast_expr.target_ctype, b->cast_expr.target_ctype) &&
                ast_equal(a->cast_expr.expr, b->cast_expr.expr);
            break;
        case AST_FOR_STMT:
            return ast_equal(a->for_stmt.init_expr, b->for_stmt.init_expr) &&
                ast_equal(a->for_stmt.update_expr, b->for_stmt.update_expr) &&
                    ast_equal(a->for_stmt.cond_expr, b->for_stmt.cond_expr) &&
                    ast_equal(a->for_stmt.body, b->for_stmt.body);
            break;
        case AST_BLOCK_STMT: {

            for (int i=0;i<a->block.statements->count;i++) {
                ASTNode * a_stmt = ASTNode_list_get(a->block.statements, i);
                ASTNode * b_stmt = ASTNode_list_get(b->block.statements, i);
                if (!ast_equal(a_stmt, b_stmt)) return false;
            }
            return true;
            break;
        }
        default:
            error("Invalid AST Node Type: %d and %d\n", get_ast_node_name(a), get_ast_node_name(b));
    }
    fprintf(stderr, "ASTNodes are not equal\n");
    return false;
}

ASTNode_list * create_node_list() {
    ASTNode_list * list = malloc(sizeof(ASTNode_list));
    ASTNode_list_init(list, free_ast);

    return list;
}

void flatten_list(ASTNode_list * list, ASTNode_list * flattened_list) {
    for (int i=0;i<list->count;i++) {
        ASTNode * astNode = ASTNode_list_get(list, i);
        if (astNode->type == AST_INITIALIZER_LIST) {
            flatten_list(astNode->initializer_list.items, flattened_list);
        } else {
            ASTNode_list_append(flattened_list, astNode);
        }
    }
}

int get_total_nested_array_elements(ASTNode * node) {
    int total = 1;
    CType * ctype = node->ctype;
    while (is_array_type(ctype)) {
        total *= ctype->array_len;
        ctype = ctype->base_type;
    }
    return total;
}

int get_array_base_element_size(ASTNode * node) {
    while (is_array_type(node->ctype)) {
        node = node->array_access.base;
    }
    return node->ctype->size;
}

bool is_assignment(ASTNode * node) {
    return node->binary.op == BINOP_ASSIGNMENT ||
                node->binary.op == BINOP_COMPOUND_ADD_ASSIGN ||
                node->binary.op == BINOP_COMPOUND_SUB_ASSIGN;
}

bool is_comparison_op(BinaryOperator op) {
    return op == BINOP_EQ || op == BINOP_NE || op == BINOP_GT || op == BINOP_GE || op == BINOP_LT || op == BINOP_LE;
}