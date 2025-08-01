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
            break;
        case CTYPE_LONG:
            emit_line(ctx, "mov rax, [%s]", reg);
            break;
        default: error("Unsupported type for load from ");
    }
}

void emit_store_to(EmitterContext * ctx, CType * t, const char * addr_reg, const char * value_reg) {
    switch (t->kind) {
        case CTYPE_CHAR:
            emit_line(ctx, "mov byte [%s], %s", addr_reg, value_reg);
            break;
        case CTYPE_SHORT:
            emit_line(ctx, "mov word [%s], %s", addr_reg, value_reg);
            break;
        case CTYPE_INT:
            emit_line(ctx, "mov dword [%s], %s", addr_reg, value_reg);
            break;
        case CTYPE_LONG:
            emit_line(ctx, "mov qword [%s], %s", addr_reg, value_reg);
            break;
        default:
            error("Unsupported type in emit_store_to: %d", t->kind);
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

void emit_line(EmitterContext * ctx, const char* fmt, ...) {
    va_list args;

    // --- 1. Write to the file
    va_start(args, fmt);
    vfprintf(ctx->out, fmt, args);
    va_end(args);
    fputc('\n', ctx->out);

    // --- 2. Echo to stdout (re-initialize args)
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fputc('\n', stdout);
}

char * make_label_text(const char * prefix, int num) {
    size_t buffer_size = strlen(prefix) + 20;
    char * label = malloc(buffer_size);
    snprintf(label, buffer_size, "%s%d", prefix, num);
    return label;
}

void emit_label(EmitterContext * ctx, const char * prefix, int num) {
    emit_line(ctx, "L%s%d:", prefix, num);
}

void emit_label_from_text(EmitterContext *ctx, const char * label) {
    emit_line(ctx, "L%s:", label);
}

char * get_data_directive(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR:   return "db";
        case CTYPE_SHORT:  return "dw";
        case CTYPE_INT:    return "dd";
        case CTYPE_LONG:   return "dq";
        case CTYPE_PTR:    return "dq";
        case CTYPE_ARRAY:  return get_data_directive(ctype->base_type);
        default:
            error("Unsupported data type for data directive: %s", ctype->kind);
    }
    return NULL;
}

char * get_reservation_directive(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "resb";
        case CTYPE_SHORT: return "resw";
        case CTYPE_INT: return "resd";
        case CTYPE_LONG: return "resq";
        case CTYPE_PTR: return "resq";
        case CTYPE_ARRAY: return get_reservation_directive(ctype->base_type);
        default:
            error("Unsupported data type");
    }
    return NULL;
}

void strip_comments(char *src, char *dst) {
    while (*src) {
        // Skip leading whitespace to peek at first non-whitespace
        char *line_start = src;
        while (*src == ' ' || *src == '\t') src++;

        if (*src == ';') {
            // full line comment: skip to end of line
            while (*src && *src != '\n') src++;
            if (*src == '\n') src++;  // skip newline too
            continue;
        }

        // copy line while stripping inline comments and trailing spaces before ;
        while (*line_start && *line_start != '\n') {
            if (*line_start == ';') {
                // backtrack over spaces in dst
                while (dst > src && (dst[-1] == ' ' || dst[-1] == '\t'))
                    dst--;
                // skip to end of line
                while (*line_start && *line_start != '\n') line_start++;
                break;
            } else {
                *dst++ = *line_start++;
            }
        }

        // copy newline if present
        if (*line_start == '\n') {
            *dst++ = *line_start++;
        }

        src = line_start;
    }

    *dst = '\0';
}