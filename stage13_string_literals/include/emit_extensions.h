//
// Created by scott on 7/13/25.
//

#ifndef EMIT_EXTENSIONS_H
#define EMIT_EXTENSIONS_H

#include "ast.h"
#include "emitter_context.h"

void emit_print_int_extension_function(EmitterContext * ctx);
void emit_print_extension_statement(EmitterContext * ctx, ASTNode * node);
void emit_print_int_extension_call(EmitterContext * ctx, ASTNode * node);


#endif //EMIT_EXTENSIONS_H
