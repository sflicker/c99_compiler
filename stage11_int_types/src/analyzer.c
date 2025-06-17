//
// Created by scott on 6/14/25.
//

#include "ast.h"
#include "analyzer.h"
#include "error.h"
#include "ctypes.h"
#include "parser_util.h"
#include "symbol_table.h"

CType * apply_integer_promotions(CType * t) {
    if (t->kind == CTYPE_CHAR || t->kind == CTYPE_SHORT) {
        return &CTYPE_INT_T;
    }
    return t;
}

CType * usual_arithmetic_conversion(CType * a, CType * b) {
    if (a == b) return a;
    if (a->rank > b->rank) return a;
    return b;
}

bool is_lvalue(ASTNode * node) {
    if (node == NULL) {
        error("node must not be null");
    }
    return node->type == AST_VAR_REF;
    // TODO more logic
}


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
    CType * saved = ctx->current_function_return_type;
    ctx->current_function_return_type = node->ctype;
    analyze(ctx, node->function_decl.body);
    ctx->current_function_return_type = saved;
    exit_scope();
}

bool is_assignment(ASTNode * node) {
    return node->binary.op == BINOP_ASSIGNMENT ||
                node->binary.op == BINOP_COMPOUND_ADD_ASSIGN ||
                node->binary.op == BINOP_COMPOUND_SUB_ASSIGN;
}
void analyze(AnalyzerContext * ctx, ASTNode * node) {
    if (!node) return;

    switch (node->type) {
        case AST_TRANSLATION_UNIT: {
            enter_scope();
            for (ASTNode_list_node * n = node->translation_unit.functions->head; n; n = n->next) {
                analyze(ctx, n->value);
            }
            exit_scope();
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
            if (node->block.introduce_scope) enter_scope();

            for (ASTNode_list_node * n = node->block.statements->head; n != NULL; n = n->next) {
                analyze(ctx, n->value);
            }

            if (node->block.introduce_scope) exit_scope();
            break;

        case AST_VAR_DECL:
            add_symbol(node->var_decl.name, node->ctype);
            if (node->var_decl.init_expr) {
                analyze(ctx, node->var_decl.init_expr);
            }
            break;

        case AST_BINARY_EXPR:
            analyze(ctx, node->binary.lhs);
            analyze(ctx, node->binary.rhs);

            if (is_assignment(node)) {
                if (!is_lvalue(node)) {
                    error("Assignment must be to an lvalue");
                }
            }

            CType * lhsCType = node->binary.lhs->ctype;
            CType * rhsCType = node->binary.rhs->ctype;

            CType * promoted_left = apply_integer_promotions(lhsCType);
            CType * promoted_right = apply_integer_promotions(rhsCType);

            CType * result_type = usual_arithmetic_conversion(promoted_left, promoted_right);

            node->ctype = result_type;

            // if (!lhsCType || !rhsCType) {
            //     error("Missing type on expression operands");
            //     break;
            // }
            //
            // node->ctype = common_type(lhsCType, rhsCType);
            break;

        case AST_UNARY_EXPR:
            analyze(ctx, node->unary.operand);
            node->ctype = node->unary.operand->ctype;
            break;

        case AST_VAR_REF:
            Symbol * symbol = lookup_symbol(node->var_ref.name);
            if (!symbol) { error("Symbol not found"); return; }
            node->ctype = symbol->ctype;
            break;

        case AST_RETURN_STMT:
            analyze(ctx, node->return_stmt.expr);
            node->ctype = node->return_stmt.expr->ctype;
            if (!ctype_equal_or_compatible(ctx->current_function_return_type,
                node->ctype)) {
                error("Function return type not compatible: expected %s, got %s",
                    ctype_to_string(ctx->current_function_return_type), ctype_to_string(node->ctype));
            }
            break;

        case AST_IF_STMT:
            analyze(ctx, node->if_stmt.cond);
            analyze(ctx, node->if_stmt.then_stmt);
            analyze(ctx, node->if_stmt.else_stmt);
            break;

        case AST_WHILE_STMT:
            analyze(ctx, node->while_stmt.cond);
            analyze(ctx, node->while_stmt.body);
            break;

        case AST_FOR_STMT:
            analyze(ctx, node->for_stmt.init_expr);
            analyze(ctx, node->for_stmt.cond_expr);
            analyze(ctx, node->for_stmt.update_expr);
            analyze(ctx, node->for_stmt.body);
            break;

        case AST_DO_WHILE_STMT:
            analyze(ctx, node->do_while_stmt.expr);
            analyze(ctx, node->do_while_stmt.body);
            break;

        case AST_LABELED_STMT:
            analyze(ctx, node->labeled_stmt.stmt);
            break;

        case AST_CASE_STMT:
            analyze(ctx, node->case_stmt.constExpression);
            analyze(ctx, node->case_stmt.stmt);
            break;

        case AST_DEFAULT_STMT:
            analyze(ctx, node->default_stmt.stmt);
            break;

        case AST_SWITCH_STMT:
            analyze(ctx, node->switch_stmt.expr);
            analyze(ctx, node->switch_stmt.stmt);
            break;

        case AST_EXPRESSION_STMT:
        case AST_ASSERT_EXTENSION_STATEMENT:
        case AST_PRINT_EXTENSION_STATEMENT:
            analyze(ctx, node->expr_stmt.expr);
            break;

        case AST_GOTO_STMT:
        case AST_INT_LITERAL:
        case AST_BREAK_STMT:
        case AST_CONTINUE_STMT:
            // DO NOTHING
            break;

        default:
            error("Unrecognized node type");
    }
}
