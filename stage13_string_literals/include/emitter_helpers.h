//
// Created by scott on 7/31/25.
//

#ifndef EMITTER_HELPERS_H
#define EMITTER_HELPERS_H

#include "ast.h"
#include "c_type.h"
#include "emitter_context.h"

char * escaped_string(const char * s);
void emit_load_from(EmitterContext * ctx, CType * t, const char * reg);
char * create_variable_reference(EmitterContext * ctx, ASTNode * node);
const char * reg_for_type(CType * ctype);
const char * mem_size_for_type(CType * ctype);
void emit_line(EmitterContext * ctx, const char* fmt, ...);
char * make_label_text(const char * prefix, int num);
void emit_label(EmitterContext * ctx, const char * prefix, int num);
void emit_label_from_text(EmitterContext *ctx, const char * label);
char * get_data_directive(CType * ctype);
char * get_reservation_directive(CType * ctype);
void strip_comments(char *src, char *dst);

#endif //EMITTER_HELPERS_H
