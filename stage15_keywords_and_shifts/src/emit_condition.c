//
// Created by scott on 10/12/25.
//

#include "emitter_context.h"
#include "ast.h"
#include "emitter.h"
#include "emit_expression.h"
#include "emit_condition.h"

#include "emitter_helpers.h"
#include "emit_stack.h"
#include "error.h"

void branch_on_truth_intptr(EmitterContext * ctx, int ltrue, int lfalse) {
    emit_line(ctx, "test rax, rax");
    emit_jump(ctx, "jne", "Lcond", ltrue);
    emit_jump(ctx, "jmp", "Lcond", lfalse);
}

void emit_condition(EmitterContext * ctx, ASTNode * node, int ltrue, int lfalse, EvalMode mode) {
    switch (node->type) {
        case AST_UNARY_EXPR: {
            if (node->unary.op == UNARY_LOGICAL_NOT) {
                emit_condition(ctx, node->unary.operand, lfalse, ltrue, mode);
                return;
            }
        }
        case AST_BINARY_EXPR: {
            switch (node->binary.op) {
                case BINOP_LOGICAL_AND: {
                    int lmid = get_label_id(ctx);
                    emit_condition(ctx, node->binary.lhs, lmid, lfalse, mode);
                    emit_label(ctx, "Lcond", lmid);
                    emit_condition(ctx, node->binary.rhs, ltrue, lfalse, mode);
                    return;
                }
                case BINOP_LOGICAL_OR: {
                    int lmid = get_label_id(ctx);
                    emit_condition(ctx, node->binary.lhs, ltrue, lmid, mode);
                    emit_label(ctx, "Lcond", lmid);
                    emit_condition(ctx, node->binary.rhs, ltrue, lfalse, mode);
                    return;
                }
                case BINOP_EQ:
                case BINOP_NE:
                case BINOP_GT:
                case BINOP_GE:
                case BINOP_LT:
                case BINOP_LE: {
                    emit_expr_to_reg(ctx, node, WANT_VALUE);
                    emit_pop(ctx, "rax");
                    branch_on_truth_intptr(ctx, ltrue, lfalse);
                    return;
                }
            }
            default: {
                emit_expr_to_reg(ctx, node, WANT_VALUE);
                emit_pop(ctx, "rax");
                branch_on_truth_intptr(ctx, ltrue, lfalse);

                return;
            }
        }
    }
}
