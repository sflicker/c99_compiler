#include "emitter.h"
#include "emitter_context.h"
#include "emit_stack.h"
#include "emitter_helpers.h"
#include "c_type.h"
#include "error.h"

void emit_push(EmitterContext * ctx, const char * reg) {
    emit_line(ctx, "push %s          ; stack += 8 (depth now %d)", reg, ctx->stack_depth + 8);
    ctx->stack_depth += 8;
}

void emit_fpush(EmitterContext * ctx, const char * xmm, FPWidth width) {
    emit_line(ctx, "sub rsp, 16      ; fpush %s (reserve 16)", xmm);
    if (width == FP64) {
        emit_line(ctx, "movsd [rsp], %s   ; store low 64 bits (double)", xmm);
    } else {
        emit_line(ctx, "movss [rsp], %s   ; store low 32 bits (float)", xmm);
    }
    ctx->stack_depth += 16;
    emit_line(ctx, "; Stack depth now %d", ctx->stack_depth);
}

void emit_push_for_type(EmitterContext * ctx, CType * ctype) {
    const char * reg = reg_for_type(ctype);
    if (is_integer_type(ctype) || (is_pointer_type(ctype) && is_integer_type(ctype->base_type))) {
        emit_push(ctx, reg);
    } else if (ctype->kind == CTYPE_FLOAT) {
        emit_fpush(ctx, reg, FP32);
    } else if (ctype->kind == CTYPE_DOUBLE) {
        emit_fpush(ctx, reg, FP64);
    } else {
        error("Unsupported data type for data directive: %s", ctype->kind);
    }

}


void emit_pop(EmitterContext * ctx, const char * reg) {
    emit_line(ctx, "pop %s           ; stack -= 8 (depth now %d)", reg, ctx->stack_depth - 8);
    ctx->stack_depth -= 8;
}

void emit_fpop(EmitterContext * ctx, const char * xmm, FPWidth width) {
    if (width == FP64) {
        emit_line(ctx, "movsd %s, [rsp]   ; load low 64 bits (double)", xmm);
    } else {
        emit_line(ctx, "movss %s, [rsp]   ; load low 32 bits (float)", xmm);
    }
    emit_line(ctx, "add rsp, 16    ; fpop %s (free 16)", xmm);
    ctx->stack_depth -= 16;
    emit_line(ctx, "; Stack depth now %d", ctx->stack_depth);
}

void emit_add_rsp(EmitterContext * ctx, int amount) {
    emit_line(ctx, "add rsp, %d    ; stack -= %d (depth now %d)", amount, amount, ctx->stack_depth - amount);
    ctx->stack_depth -= amount;
}

void emit_sub_rsp(EmitterContext * ctx, int amount) {
    emit_line(ctx, "sub rsp, %d    ; stack += %d (depth now %d)", amount, amount, ctx->stack_depth + amount);
    ctx->stack_depth += amount;
}

void emit_pop_for_type(EmitterContext * ctx, CType * ctype) {
    const char * reg = reg_for_type(ctype);
    if (is_integer_type(ctype) || (is_pointer_type(ctype) && is_integer_type(ctype->base_type))) {
        emit_pop(ctx, "rax");
    } else if (ctype->kind == CTYPE_FLOAT) {
        emit_fpop(ctx, reg, FP32);
    } else if (ctype->kind == CTYPE_DOUBLE) {
        emit_fpop(ctx, reg, FP64);
    } else if (is_array_type(ctype)) {
        CType * base_type = get_base_type(ctype);
        if (is_integer_type(base_type)) {
            emit_pop(ctx, "rax");
        } else if (base_type->kind == CTYPE_FLOAT) {
            emit_fpop(ctx, reg, FP32);
        } else if (base_type->kind == CTYPE_DOUBLE) {
            emit_fpop(ctx, reg, FP64);
        }
    } else {
        error("Unsupported data type for data directive: %s", ctype->kind);
    }
}
