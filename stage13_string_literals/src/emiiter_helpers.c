//
// Created by scott on 7/31/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "list_util.h"
#include "c_type.h"
#include "ast.h"
#include "emitter.h"

#include "token.h"
#include "util.h"
#include "error.h"
#include "emitter_context.h"
#include "emit_extensions.h"
#include "symbol.h"
#include "emitter_helpers.h"

char * escaped_string(const char * s) {
    size_t len = strlen(s);
    char *output = malloc(len * 4 + 3);  // over allocate
    char *out = output;

    *out++ = '"';           // start with quote
    for (const char * p = s; *p; p++) {
        unsigned char c = (unsigned char) *p;
        switch (c) {
            case '\n': *out++ = '\\'; *out++ = 'n'; break;
            case '\r': *out++ = '\\'; *out++ = 'r'; break;
            case '\t': *out++ = '\\'; *out++ = 't'; break;
            case '\\': *out++ = '\\'; *out++ = '\\'; break;
            case '\"': *out++ = '\\'; *out++ = '\"'; break;
            default:
                if (c < 32 || c >= 127) {
                    sprintf(out, "\\x%02x", c);
                    out += 4;
                } else {
                    *out++ = c;
                }
        }
    }

    *out++ = '"';    // closing quote
    *out = '\0';
    return output;
}

void emit_load_from(EmitterContext * ctx, CType * t, const char * reg) {
    switch (t->kind) {
        case CTYPE_CHAR:
            emit_line(ctx, "movsx eax, byte [%s]", reg);
            break;
        case CTYPE_SHORT:
            emit_line(ctx, "movsx eax, word [%s]", reg);
            break;
        case CTYPE_INT:
            emit_line(ctx, "mov eax, [%s]", reg);
        case CTYPE_LONG:
            emit_line(ctx, "mov rax, [%s]", reg);
            break;
        error("Unsupported type for load");
    }
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

const char * reg_for_type(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "al";
        case CTYPE_SHORT: return "ax";
        case CTYPE_INT: return "eax";
        case CTYPE_LONG: return "rax";
        case CTYPE_PTR: return "rax";
        default: error("Unsupported type");
    }
    return NULL;
}

const char * mem_size_for_type(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "BYTE";
        case CTYPE_SHORT: return "WORD";
        case CTYPE_INT: return "DWORD";
        case CTYPE_LONG: return "QWORD";
        case CTYPE_PTR:  return "QWORD";
        default:
            error("Unsupported type");
    }
    return NULL;
}
