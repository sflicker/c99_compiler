//
// Created by scott on 7/31/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "list_util.h"
#include "c_type.h"
#include "ast.h"
#include "emitter.h"
#include "emit_stack.h"

#include "token.h"
#include "util.h"
#include "error.h"
#include "emitter_context.h"
#include "emit_extensions.h"
#include "symbol.h"
#include "emitter_helpers.h"
#include "emit_stack.h"


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
        case CTYPE_PTR:
            emit_line(ctx, "mov rax, [%s]", reg);
            break;
        case CTYPE_FLOAT:
            emit_line(ctx, "movss xmm0, [%s]", reg);
            break;
        case CTYPE_DOUBLE:
            emit_line(ctx, "movsd xmm0, [%s]", reg);
            break;
        default: error("Unsupported type %s for load from ");
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
        case CTYPE_FLOAT:
            emit_line(ctx, "movss [%s], %s", addr_reg, value_reg);
            break;
        case CTYPE_DOUBLE:
            emit_line(ctx, "movsd [%s], %s", addr_reg, value_reg);
            break;
        default:
            error("Unsupported type in emit_store_to: %d", t->kind);
    }
}

const char * stack_reg_for_type(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "rax";
        case CTYPE_SHORT: return "rax";
        case CTYPE_INT: return "rax";
        case CTYPE_LONG: return "rax";
        case CTYPE_FLOAT: return "xmm0";
        case CTYPE_DOUBLE: return "xmm0";
        case CTYPE_PTR: return "rax";
        default: error("Unsupported type");
    }
    return NULL;

}

const char * reg_for_type(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "al";
        case CTYPE_SHORT: return "ax";
        case CTYPE_INT: return "eax";
        case CTYPE_LONG: return "rax";
        case CTYPE_FLOAT: return "xmm0";
        case CTYPE_DOUBLE: return "xmm0";
        case CTYPE_PTR: return "rax";
        case CTYPE_ARRAY: return reg_for_type(ctype->base_type);
        default: error("Unsupported type");
    }
    return NULL;
}

const char * aux_reg_for_type(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "cl";
        case CTYPE_SHORT: return "cx";
        case CTYPE_INT: return "ecx";
        case CTYPE_LONG: return "rcx";
        case CTYPE_FLOAT: return "xmm1";
        case CTYPE_DOUBLE: return "xmm1";
        case CTYPE_PTR: return "rcx";
        default: error("Unsupported type");
    }
    return NULL;
}
const char * mov_instruction_for_type(CType * ctype) {
    switch (ctype->kind) {
        case CTYPE_CHAR: return "mov";
        case CTYPE_SHORT: return "mov";
        case CTYPE_INT: return "mov";
        case CTYPE_LONG: return "mov";
        case CTYPE_FLOAT: return "movss";
        case CTYPE_DOUBLE: return "movsd";
        case CTYPE_PTR: return "mov";
        case CTYPE_ARRAY: return mov_instruction_for_type(ctype->base_type);
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
        case CTYPE_FLOAT: return "DWORD";
        case CTYPE_DOUBLE: return "QWORD";
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

void strip_comments_multiline(char * src, char * dst) {
    char line[256];
    char stripped[256];
    char * p = src;
    while (*p) {
        char * q = strchr(p, '\n');
        if (!q) q = p + strlen(p);   // last line without newline
        size_t len = q - p;
        memcpy(line, p, len);
        line[len] = '\0';

        strip_comments(line, stripped);
        if (strlen(stripped) > 0) {
            memcpy(dst, stripped, strlen(stripped));
            dst[strlen(stripped)] = '\n';
            p = q + 1;
            dst += strlen(dst);
        } else {
            p = q + 1;
        }
    }
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


void emit_leave(EmitterContext *ctx) {
    emit_line(ctx, "leave      ; restore rbp; stack -= 8 (depth now %d)", ctx->stack_depth - 8);
    ctx->stack_depth -= 8;
}

void emit_pointer_arithmetic(EmitterContext * ctx, CType * c_type) {
    emit_line(ctx, "; emitting pointer arithmetic");
    int size = c_type->base_type ? c_type->base_type->size : 1;
    emit_pop(ctx, "rcx");     // offset
    emit_pop(ctx, "rax");     // base
    if (size >= 1) {
        emit_line(ctx, "imul rcx, %d", size);
    }
    emit_line(ctx, "add rax, rcx");
    emit_push(ctx, "rax");
}

void emit_binary_op(EmitterContext * ctx, BinaryOperator op) {
    switch(op) {
        case BINOP_ADD:
            emit_line(ctx, "add eax, ecx");
            break;
        case BINOP_SUB:
            emit_line(ctx, "sub ecx, eax");
            emit_line(ctx, "mov eax, ecx");
            break;
        case BINOP_MUL:
            emit_line(ctx, "imul eax, ecx");
            break;
        default:
            error("Unsupported binary operator: %s", token_type_name(op));
    }
}

void emit_float_literal(EmitterContext * ctx, const char * label, float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));

    emit_line(ctx, "align 4");

    // Emit the labeled constant. NASM/YASM syntax: dd = 32-bit data.
    // We use hex to lock the exact bit pattern. Include a comment for readability.
    emit_line(ctx, "%s: dd 0x%08X    ; %f", label, bits, (double)value);
}

void emit_double_literal(EmitterContext *ctx, const char *label, double value) {
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));

    emit_line(ctx, "align 8");
    emit_line(ctx, "%s: dq 0x%016llX    ; %f",
              label, (unsigned long long)bits, value);
}

void emit_string_literal(EmitterContext * ctx, const char * label, const char * literal) {
    size_t literal_len = strlen(literal);
    char buffer[1024];
    buffer[0] = '\0';
    int written = snprintf(buffer, 1024, "%s: db ", label);
    bool in_quotes = false;
    for (int i = 0; i < literal_len; i++) {
        char c = literal[i];

        bool printable = (c >= 32 && c <= 126 && c != '"');

        if (printable) {
            if (!in_quotes) {
                if (i > 0) {
                    written += snprintf(buffer + written, 1024 - written, ", ");
                }
                written += snprintf(buffer + written, 1024 - written, "\"");
                in_quotes = true;
            }
            written += snprintf(buffer + written, 1024 - written, "%c", c);
        } else {
            if (in_quotes) {
                written += snprintf(buffer + written, 1024 - written, "\"");
                in_quotes = false;
            }
            if (i > 0) {
                written += snprintf(buffer + written, 1024 - written, ", ");
            }
            written += snprintf(buffer + written, 1024 - written, "%u", c);
        }
    }

    if (in_quotes) {
        written += snprintf(buffer + written, 1024 - written, "\"");
    }

    written += snprintf(buffer + written, 1024 - written, ", 0");
    emit_line(ctx, "%s", buffer);
}

int get_offset(EmitterContext * ctx, ASTNode * node) {
    if (node->type == AST_VAR_DECL || node->type == AST_VAR_REF_EXPR) {
        return node->symbol->info.var.offset;
    }
    if (node->type == AST_ARRAY_ACCESS) {
        return get_offset(ctx, node->array_access.base);
        //       return node->array_access.base->symbol->info.var.offset;
    }
    return 0;
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

FPWidth getFPWidthFromCType(CType * ctype) {
    if (ctype->kind == CTYPE_FLOAT) {
        return FP32;
    }
    else if (ctype->kind == CTYPE_DOUBLE) {
        return FP64;
    }
    error("Unsupported type: %s", ctype->kind);
    return FPWIDTHUNKN;
}

char * get_fp_binop(ASTNode * node) {
    switch (node->binary.op) {
        case BINOP_ADD: return node->ctype->kind == CTYPE_FLOAT ? "addss" : "addsd"; break;
        case BINOP_SUB: return node->ctype->kind == CTYPE_FLOAT ? "subss" : "subsd"; break;
        case BINOP_MUL: return node->ctype->kind == CTYPE_FLOAT ? "mulss" : "mulsd"; break;
        case BINOP_DIV: return node->ctype->kind == CTYPE_FLOAT ? "divss" : "divsd"; break;
        default: error("Unsupported binary op in emitter");
            return NULL;
    }
}