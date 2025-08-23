//
// Created by scott on 8/9/25.
//

#ifndef _EMIT_EXPRESSION_H
#define _EMIT_EXPRESSION_H

#include "ast.h"
#include "emitter_context.h"

void emit_int_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode);

#endif //_EMIT_EXPRESSION_H