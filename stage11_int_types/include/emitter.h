#ifndef EMITTER_H
#define EMITTER_H

#include "ast.h"

void emit(ASTNode * program, const char * output_file);

void emit_binary_op(FILE * out, BinaryOperator op);
void emit_binary_comparison(FILE * out, ASTNode * node);
void emit_logical_and(FILE * out, ASTNode * node);
void emit_logical_or(FILE * out, ASTNode * node);
void emit_assignment(FILE * out, ASTNode* node);
void emit_add_assignment(FILE *out, ASTNode * node);
void emit_sub_assignment(FILE *out, ASTNode * node);
void emit_binary_div(FILE* out, ASTNode * node);
void emit_binary_mod(FILE *out, ASTNode * node);
#endif