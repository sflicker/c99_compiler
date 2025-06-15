//
// Created by scott on 6/14/25.
//

#include "ast.h"
#include "analyzer.h"
#include "error.h"
#include "ctypes.h"
#include "symbol_table.h"

CType * common_type(CType *a, CType *b) {
    if (a->kind == CTYPE_LONG && b->kind == CTYPE_LONG) return &CTYPE_LONG_T;
    if (a->kind == CTYPE_INT && b->kind == CTYPE_INT) return &CTYPE_INT_T;
    if (a->kind == CTYPE_SHORT && b->kind == CTYPE_SHORT) return &CTYPE_INT_T;  // promote to int
    if (a->kind == CTYPE_CHAR && b->kind == CTYPE_CHAR) return &CTYPE_INT_T;  // promote to int
    return NULL;
}

// paramList should be made of a list of VarDecl
CTypePtr_list * paramListToTypeList(const ASTNode_list * param_list) {
    CTypePtr_list * typeList = malloc(sizeof(CTypePtr_list));
    CTypePtr_list_init(typeList, free_ctype);
    for (ASTNode_list_node * n = param_list->head; n != NULL; n = n->next) {
    //    add_symbol(n->value->var_decl.name, n->value->ctype);
        CTypePtr_list_append(typeList, n->value->ctype);
    }
    return typeList;
}

// argList should be made of a list of expressions
CTypePtr_list * argListToTypeList(const ASTNode_list * arg_list) {
    CTypePtr_list * typeList = malloc(sizeof(CTypePtr_list));
    CTypePtr_list_init(typeList, free_ctype);
    for (ASTNode_list_node * n = arg_list->head; n != NULL; n = n->next) {
//        add_symbol(n->value->var_decl.name, n->value->ctype);
        CTypePtr_list_append(typeList, n->value->ctype);
    }
    return typeList;
}


void handle_function_declaration(ASTNode * node) {

    enter_scope();
    CTypePtr_list * typeList = paramListToTypeList(node->function_decl.param_list);
    // CTypePtr_list * typeList = malloc(sizeof(CTypePtr_list));
    // CTypePtr_list_init(typeList, free_ctype);
    for (ASTNode_list_node * n = node->function_decl.param_list->head; n != NULL; n = n->next) {
         add_symbol(n->value->var_decl.name, n->value->ctype);
    //     CTypePtr_list_append(typeList, node->ctype);
    }
    add_function_symbol(node->function_decl.name, node->ctype,
        node->function_decl.param_count, typeList);
    analyze(node->function_decl.body, false);
    exit_scope();
}

void analyze(ASTNode * node, bool make_new_scope) {
    if (!node) return;

    switch (node->type) {
        case AST_TRANSLATION_UNIT: {
            for (ASTNode_list_node * n = node->translation_unit.functions->head; n; n = n->next) {
                analyze(n->value, true);
            }
            break;
        }
        case AST_FUNCTION_DECL: {
            handle_function_declaration(node);
            break;
        }

        case AST_FUNCTION_CALL: {
            FunctionSymbol * functionSymbol = lookup_function_symbol(node->function_call.name);
            if (!functionSymbol) {
                error("Function symbol not found");
                return;
            }
            if (functionSymbol->param_count != node->function_call.arg_list->count) {
                error("Function arguments count not equal");
            }
            if (!ctype_lists_equal(functionSymbol->param_types, argListToTypeList( node->function_call.arg_list))) {
                error("Function parameter types not equal");
            }
            break;
        }

        case AST_BLOCK:
            if (make_new_scope) enter_scope();

            for (ASTNode_list_node * n = node->block.statements->head; n != NULL; n = n->next) {
                analyze(n->value, true);
            }

            if (make_new_scope) exit_scope();
            break;

        case AST_VAR_DECL:
            add_symbol(node->var_decl.name, node->ctype);
            if (node->var_decl.init_expr) {
                analyze(node->var_decl.init_expr, false);
            }
            break;

        case AST_BINARY_EXPR:
            analyze(node->binary.lhs, true);
            analyze(node->binary.rhs, true);

            CType * lhsCType = node->binary.lhs->ctype;
            CType * rhsCType = node->binary.rhs->ctype;

            if (!lhsCType || !rhsCType) {
                error("Missing type on break expression operands");
                break;
            }

            node->ctype = common_type(lhsCType, rhsCType);
            break;

        case AST_UNARY_EXPR:
            analyze(node->unary.operand, true);
            node->ctype = node->unary.operand->ctype;
            break;

        case AST_VAR_REF:
            Symbol * symbol = lookup_symbol(node->var_ref.name);
            if (!symbol) { error("Symbol not found"); return; }
            node->ctype = symbol->ctype;

        default:
            error("Unrecognized node type");
    }
}
