//
// Created by scott on 6/14/25.
//

#include "ast.h"
#include "analyzer.h"
#include "error.h"
#include "ctypes.h"
#include "symbol_table.h"


void handle_function_declaration(AnalyzerContext * ctx, ASTNode * node) {

    enter_scope();
    CTypePtr_list * typeList = astNodeListToTypeList(node->function_decl.param_list);
    // CTypePtr_list * typeList = malloc(sizeof(CTypePtr_list));
    // CTypePtr_list_init(typeList, free_ctype);
    if (node->function_decl.param_list != NULL) {
        for (ASTNode_list_node * n = node->function_decl.param_list->head; n != NULL; n = n->next) {
            add_symbol(n->value->var_decl.name, n->value->ctype);
            //     CTypePtr_list_append(typeList, node->ctype);
        }
    }
    add_function_symbol(node->function_decl.name, node->ctype,
        node->function_decl.param_count, typeList);
    ctx->current_function_return_type = node->ctype;
    analyze(node->function_decl.body, false);
    exit_scope();
}

void analyze(AnalyzerContext * ctx, ASTNode * node) {
    if (!node) return;

    switch (node->type) {
        case AST_TRANSLATION_UNIT: {
            for (ASTNode_list_node * n = node->translation_unit.functions->head; n; n = n->next) {
                analyze(ctx, n->value);
            }
            break;
        }
        case AST_FUNCTION_DECL: {
            handle_function_declaration(ctx, node);
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
            if (!ctype_lists_equal(functionSymbol->param_types, astNodeListToTypeList( node->function_call.arg_list))) {
                error("Function parameter types not equal");
            }
            break;
        }

        case AST_BLOCK:
            if (ctx->make_new_scope) enter_scope();

            for (ASTNode_list_node * n = node->block.statements->head; n != NULL; n = n->next) {
                analyze(ctx, n->value);
            }

            if (ctx->make_new_scope) exit_scope();
            break;

        case AST_VAR_DECL:
            add_symbol(node->var_decl.name, node->ctype);
            if (node->var_decl.init_expr) {
                ctx->make_new_scope = false;
                analyze(ctx, node->var_decl.init_expr);
            }
            break;

        case AST_BINARY_EXPR:
            ctx->make_new_scope = true;
            analyze(ctx, node->binary.lhs);

            ctx->make_new_scope = true;
            analyze(ctx, node->binary.rhs);

            CType * lhsCType = node->binary.lhs->ctype;
            CType * rhsCType = node->binary.rhs->ctype;

            if (!lhsCType || !rhsCType) {
                error("Missing type on expression operands");
                break;
            }

            node->ctype = common_type(lhsCType, rhsCType);
            break;

        case AST_UNARY_EXPR:
            ctx->make_new_scope = true;
            analyze(ctx, node->unary.operand);
            node->ctype = node->unary.operand->ctype;
            break;

        case AST_VAR_REF:
            Symbol * symbol = lookup_symbol(node->var_ref.name);
            if (!symbol) { error("Symbol not found"); return; }
            node->ctype = symbol->ctype;
            break;

        case AST_RETURN_STMT:
            ctx->make_new_scope = true;
            analyze(ctx, node->return_stmt.expr);
            node->ctype = node->return_stmt.expr->ctype;
            if (!ctype_equal_or_compatible(ctx->current_function_return_type,
                node->ctype)) {
                error("Function return type not compatible");
            }
            break;


        case AST_INT_LITERAL:
            // DO NOTHING
            break;

        default:
            error("Unrecognized node type");
    }
}
