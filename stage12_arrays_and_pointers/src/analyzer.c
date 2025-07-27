//
// Created by scott on 6/14/25.
//

#include "ast.h"
#include "analyzer.h"
#include "error.h"
#include "c_type.h"
#include "parser_util.h"
#include "symbol.h"
#include "symbol_table.h"

//int local_offset = -8;
int local_offset = 0;
int param_offset = 16;
int function_local_storage = 0;

void reset_size_and_offsets() {
    local_offset = 0;
    param_offset = 16;
    function_local_storage = 0;
}

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
    switch (node->type) {
        case AST_VAR_REF_EXPR:
        case AST_ARRAY_ACCESS:
            return true;

        case AST_UNARY_EXPR:
            if (node->unary.op == UNARY_DEREF) {
                return true;
            }

        default:
            return false;
    }
}


int astNodeListLength(ASTNode_list * ast_nodes) {
    return (ast_nodes != NULL) ? ast_nodes->count : 0;
}

void handle_function_declaration(AnalyzerContext * ctx, ASTNode * node) {

    enter_scope();
    reset_size_and_offsets();
    Symbol_list * symbol_list = NULL;
    if (node->function_decl.param_list != NULL) {
        symbol_list = malloc(sizeof(Symbol_list));
        Symbol_list_init(symbol_list, free_symbol);;
        for (ASTNode_list_node * n = node->function_decl.param_list->head; n != NULL; n = n->next) {
            Symbol * symbol = create_symbol(n->value->var_decl.name, SYMBOL_VAR, n->value->ctype, n->value);
            symbol->info.var.offset = param_offset;
            symbol->info.var.storage = STORAGE_PARAMETER;
            param_offset += 8;
            add_symbol(symbol);
            n->value->symbol = symbol;
            Symbol_list_append(symbol_list, symbol);
        }
    }
    Symbol * symbol = create_symbol(node->function_decl.name, SYMBOL_FUNC, node->ctype, node);
    symbol->info.func.num_params = astNodeListLength(node->function_decl.param_list);
    symbol->info.func.params_symbol_list = symbol_list;
    add_global_symbol(symbol);
    node->symbol = symbol;

    CType * saved = ctx->current_function_return_type;
    ctx->current_function_return_type = node->ctype;
    analyze(ctx, node->function_decl.body);
    ctx->current_function_return_type = saved;
    node->function_decl.size = function_local_storage;
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
            for (ASTNode_list_node * n = node->translation_unit.globals->head; n != NULL; n = n->next) {
                analyze(ctx, n->value);
            }
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

        case AST_FUNCTION_CALL_EXPR: {
            Symbol * functionSymbol = lookup_table_symbol(getGlobalScope(), node->function_call.name);
            if (!functionSymbol) {
                error("function symbol not found - %s", node->function_call.name);
                return;
            }

            size_t arg_index = 0;
            if (node->function_call.arg_list != NULL) {
                for (ASTNode_list_node * arg = node->function_call.arg_list->head; arg != NULL; arg = arg->next) {
                    analyze(ctx, arg->value);
                    CType * arg_type = arg->value->ctype;

                    if (arg_index >= functionSymbol->info.func.num_params) {
                        error("Too many arguments for function %s", node->function_call.name);
                        return;
                    } else if (!ctype_equal_or_compatible(arg_type, Symbol_list_get(functionSymbol->info.func.params_symbol_list, arg_index)->ctype)) {
                        error("Type mismatch for function %s", node->function_call.name);
                        return;
                    }

                    arg_index++;
                }
            }

            if (arg_index < functionSymbol->info.func.num_params) {
                error("Too few arguments to function %s", node->function_call.name);
                return;
            }

            node->symbol = functionSymbol;
            node->ctype = functionSymbol->node->ctype;
            break;
        }

        case AST_BLOCK_STMT:
            if (node->block.introduce_scope) enter_scope();

            for (ASTNode_list_node * n = node->block.statements->head; n != NULL; n = n->next) {
                analyze(ctx, n->value);
            }

            if (node->block.introduce_scope) exit_scope();
            break;

        case AST_VAR_DECL:
            Symbol * symbol = create_symbol(node->var_decl.name, SYMBOL_VAR, node->ctype, node);
            if (node->var_decl.is_param) {
                symbol->info.var.storage = STORAGE_PARAMETER;
                // handled in function
                //symbol->info.var.offset = param_offset;
                //symbol->info.var.storage = STORAGE_PARAMETER;
            }
            else if (node->var_decl.is_global) {
                symbol->info.var.storage = STORAGE_GLOBAL;
            }
            else {
                symbol->info.var.offset = local_offset - node->ctype->size;
                symbol->info.var.storage = STORAGE_LOCAL;
                // local_offset -= 8;
                // function_local_storage += 8;
                local_offset -= node->ctype->size;
                function_local_storage += node->ctype->size;

            }
            add_symbol(symbol);
            node->symbol = symbol;
            if (node->var_decl.init_expr) {
                analyze(ctx, node->var_decl.init_expr);
            }
            break;

        case AST_BINARY_EXPR:
            analyze(ctx, node->binary.lhs);
            analyze(ctx, node->binary.rhs);

            if (is_assignment(node)) {
                if (!is_lvalue(node->binary.lhs)) {
                    error("Assignment must be to an lvalue");
                }
            }

            CType * lhsCType = node->binary.lhs->ctype;
            CType * rhsCType = node->binary.rhs->ctype;

            CType * promoted_left = is_integer_type(lhsCType)
                ? apply_integer_promotions(lhsCType) : lhsCType;
            CType * promoted_right = is_integer_type(rhsCType)
                ? apply_integer_promotions(rhsCType) : rhsCType;

            CType * result_type = usual_arithmetic_conversion(promoted_left, promoted_right);

            node->ctype = result_type;

            break;

        case AST_UNARY_EXPR:
            analyze(ctx, node->unary.operand);
            if (node->unary.op == UNARY_DEREF) {
                if (node->unary.operand->ctype->kind == CTYPE_PTR) {
                    node->ctype = node->unary.operand->ctype->base_type;
                    break;
                }
            }
            node->ctype = node->unary.operand->ctype;
            break;

        case AST_VAR_REF_EXPR: {
            Symbol * symbol = lookup_symbol(node->var_ref.name);
            if (!symbol) { error("Symbol not found"); return; }
            node->symbol = symbol;
            // if (symbol->ctype->kind == CTYPE_ARRAY) {
            //      node->ctype = make_pointer_type(symbol->ctype->base_type);
            // }
            // else {
            //     node->ctype = symbol->ctype;
            // }
            node->ctype = symbol->ctype;

            break;
        }

        case AST_RETURN_STMT: {

            analyze(ctx, node->return_stmt.expr);
            node->ctype = node->return_stmt.expr->ctype;
            if (!ctype_equal_or_compatible(ctx->current_function_return_type,
                node->ctype)) {
                char buf_expected[128];
                buf_expected[0] = '\0';
                ctype_to_cdecl(ctx->current_function_return_type, buf_expected, sizeof(buf_expected));
                char buf_actual[128];
                buf_actual[0] = '\0';
                ctype_to_cdecl(node->ctype, buf_actual, sizeof(buf_actual));
                error("Function return type not compatible: expected %s, got %s",
                    buf_expected, buf_actual);
                }
            break;
        }

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
            enter_scope();
            analyze(ctx, node->for_stmt.init_expr);
            analyze(ctx, node->for_stmt.cond_expr);
            analyze(ctx, node->for_stmt.update_expr);
            analyze(ctx, node->for_stmt.body);
            exit_scope();
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

        case AST_CAST_EXPR:
            analyze(ctx, node->cast_expr.expr);
            node->ctype = node->cast_expr.target_type;
            break;

        case AST_ARRAY_ACCESS:
            analyze(ctx, node->array_access.base);
            analyze(ctx, node->array_access.index);

//            CType * base_type = decay_if_array(node->array_access.base->ctype);
            CType * base_type = node->array_access.base->ctype;
            CType * index_type = node->array_access.index->ctype;

            if (!(is_pointer_type(base_type) || is_array_type(base_type))) {
                error("Array base must be a pointer or array");
            }

            if (!is_integer_type(index_type)) {
                error("Array index must be an integer");
            }

            node->ctype = base_type->base_type;
            break;

        case AST_INITIALIZER_LIST:
            // TODO
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
            error("Unrecognized node type - %s", get_ast_node_name(node));
    }
}
