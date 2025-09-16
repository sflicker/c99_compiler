#ifndef __EMIT_STACK_
#define __EMIT_STACK_

#include "c_type.h"
#include "emitter_context.h"

void emit_push(EmitterContext * ctx, const char * reg);
void emit_fpush(EmitterContext * ctx, const char * xmm, FPWidth width);
void emit_push_for_type(EmitterContext * ctx, CType * ctype);
void emit_pop_for_type(EmitterContext * ctx, CType * ctype);
void emit_pop(EmitterContext * ctx, const char * reg);
void emit_fpop(EmitterContext * ctx, const char * xmm, FPWidth width);
void emit_add_rsp(EmitterContext * ctx, int amount);
void emit_sub_rsp(EmitterContext * ctx, int amount);


#endif