//
// Created by scott on 8/4/25.
//
#include "ast.h"
#include "c_type.h"
#include "emitter.h"
#include "emitter_context.h"
#include "emitter_helpers.h"
#include "error.h"
#include "symbol.h"

void emit_array_access_addr(EmitterContext * ctx, ASTNode * node);

int get_offset(EmitterContext * ctx, ASTNode * node) {
    if (node->type == AST_VAR_DECL || node->type == AST_VAR_REF_EXPR) {
        return node->symbol->info.var.offset;
    }
    if (node->type == AST_ARRAY_ACCESS) {
        return get_offset(ctx, node->array_access.base);
        //       return node->array_access.base->symbol->info.var.offset;
    }
    return 0;
}

char * create_variable_reference(EmitterContext * ctx, ASTNode * node) {
    if (is_global_var(ctx, node)) {
        const char * name = get_var_name(ctx, node);
        int size = snprintf(NULL, 0, "[rel %s]", name) + 1;
        char * label = malloc(size);
        snprintf(label, size, "[rel %s]", name);
        return label;
    }
    else {
        int offset = get_offset(ctx, node);
        int size = snprintf(NULL, 0, "[rbp%+d]", offset) + 1;
        char * label = malloc(size);
        snprintf(label, size, "[rbp%+d]", offset);
        return label;
    }
}

void emit_addr(EmitterContext * ctx, ASTNode * node) {
    switch (node->type) {
        case AST_VAR_DECL:
        case AST_VAR_REF_EXPR: {

            if (is_global_var(ctx, node)) {
                emit_line(ctx, "lea rax, [rel %s]", node->symbol->name);
                emit_push(ctx, "rax");
                break;
            }
            if (node->symbol->storage == STORAGE_LOCAL) {
//                int offset = node->symbol->info.var.offset;
                int offset = get_offset(ctx, node);
                emit_line(ctx, "lea rax, [rbp%+d]", offset);
                emit_push(ctx, "rax");
                break;
            }
            // if (node->symbol->ctype->kind == CTYPE_ARRAY) {
            //     emit_line(ctx, "lea rax, [rbp-%d]", abs(node->symbol->info.var.offset));
            //     emit_push(ctx, "rax");
            // }
            // else if (node->symbol->node->var_decl.is_global) {
            //     emit_line(ctx, "mov %s, [rel %s] ", reg_for_type(node->ctype), node->symbol->name);
            //     emit_push(ctx, "rax");
            // } else {
            //     int offset = node->symbol->info.var.offset;
            //     emit_line(ctx, "mov %s, [rbp%+d]", reg_for_type(node->ctype), offset);
            //     emit_push(ctx, "rax");
            // }


            // char * label = create_variable_reference(ctx, node);
            // emit_line(ctx, "lea rcx, %s", label);
            // emit_push(ctx, "rcx");
            //
            // free(label);
            break;
        }
        case AST_UNARY_EXPR:
            if (node->unary.op == UNARY_DEREF) {
                emit_expr(ctx, node->unary.operand);
            }
            else {
                error("Unsupported unary operator in emit_addr");
            }
            break;
        case AST_ARRAY_ACCESS: {
            emit_array_access_addr(ctx, node);
            break;
        }
        case AST_STRING_LITERAL: {
            char * label = node->string_literal.label;
            emit_line(ctx, "lea rax, [%s]", label);
            emit_push(ctx, "rax");

            break;
        }

        default:
            error("Unexpected node type %s", get_ast_node_name(node));

    }
}

void emit_array_access_addr(EmitterContext * ctx, ASTNode * node) {
    CType * base_type = node->array_access.base->ctype;

    emit_line(ctx, "; emitting array base");
    emit_expr(ctx, node->array_access.base);

    emit_line(ctx, "; emitting array index");
    emit_expr(ctx, node->array_access.index);

    if (base_type->kind == CTYPE_PTR) {
        emit_pointer_arithmetic(ctx, node->array_access.base->ctype);
    }
    else if (base_type->kind == CTYPE_ARRAY) {
        emit_line(ctx, "; emiiting array base + index*size");
        int size = base_type->base_type ? base_type->base_type->size : 1;
        emit_pop(ctx, "rcx");
        emit_pop(ctx, "rax");
        emit_line(ctx, "imul rcx, %d", size);
        emit_line(ctx, "add rax, rcx");
        emit_push(ctx, "rax");
    }
    else {
        error("Unsupported types for array access");
    }
}
