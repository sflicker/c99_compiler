//
// Created by scott on 10/12/25.
//

#include "emitter_context.h"
#include "ast.h"
#include "emit_condition.h"

#include "emitter_helpers.h"


void emit_condition(EmitterContext * ctx, ASTNode * node, int ltrue, int lfalse) {
    switch (node->type) {
        case AST_UNARY_EXPR: {
            if (node->unary.op == UNARY_NOT) {
                emit_condition(ctx, node->unary.operand, lfalse, ltrue);
                return;
            }
        }
        case AST_BINARY_EXPR: {
            switch (node->binary.op) {
                case BINOP_LOGICAL_AND: {
                    int lmid = get_label_id(ctx);
                    emit_condition(ctx, node->binary.lhs, lmid, lfalse);
                    emit_label(ctx, "Lcond", lmid);
                    emit_condition(ctx, node->binary.rhs, ltrue, lfalse);
                    return;
                }
                case BINOP_LOGICAL_OR: {
                    int lmid = get_label_id(ctx);
                    emit_condition(ctx, node->binary.lhs, ltrue, lmid);
                    emit_label(ctx, "Lcond", lmid);
                    emit_condition(ctx, node->binary.rhs, ltrue, lfalse);
                    return;
                }
            }
        }
    }
}
