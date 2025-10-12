//
// Created by scott on 10/12/25.
//

#ifndef _EMIT_CONDITION_H
#define _EMIT_CONDITION_H
#include "ast.h"
#include "emitter_context.h"

void emit_condition(EmitterContext * ctx, ASTNode * node, int ltrue, int lfalse);

#endif// _EMIT_CONDITION_H