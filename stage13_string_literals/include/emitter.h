#ifndef EMITTER_H
#define EMITTER_H

#include "ast.h"
#include "emitter_context.h"

const char * get_break_label(EmitterContext * ctx);
const char * get_continue_label(EmitterContext * ctx);
const char * reg_for_type(CType * ctype);
const char * mem_size_for_type(CType * ctype);

void emit(EmitterContext * ctx, ASTNode * program);
void emit_tree_node(EmitterContext * ctx, ASTNode * node);
void emit_binary_expr(EmitterContext * ctx, ASTNode *node);
void emit_expr(EmitterContext * ctx, ASTNode * node);
//void emit_addr(EmitterContext * ctx, ASTNode * node);
void emit_binary_add(EmitterContext * ctx, ASTNode * node);
void emit_translation_unit(EmitterContext * ctx, ASTNode * node);
void emit_header(EmitterContext * ctx);
void emit_rodata(EmitterContext * ctx, ASTNode_list * string_literals);
void emit_text_section_header(EmitterContext * ctx);
void emit_data_section_header(EmitterContext * ctx);
void emit_bss_section_header(EmitterContext * ctx);
void emit_jump(EmitterContext * ctx, const char * op, const char * prefix, int num);
void emit_jump_from_text(EmitterContext * ctx, const char * op, const char * label);
void emit_assert_extension_statement(EmitterContext * ctx, ASTNode * node);
//void emit_assert_extension_call(EmitterContext * ctx, ASTNode * node);

void emit_array_access(EmitterContext * ctx, ASTNode * node);
//void emit_print_int_extension_function(EmitterContext * ctx);

void emit_binary_expr(EmitterContext * ctx, ASTNode *node);
void emit_binary_op(EmitterContext * ctx, BinaryOperator op);
void emit_binary_comparison(EmitterContext * ctx, ASTNode * node);
void emit_logical_and(EmitterContext * ctx, ASTNode * node);
void emit_logical_or(EmitterContext * ctx, ASTNode * node);
void emit_assignment(EmitterContext * ctx, ASTNode* node);
void emit_add_assignment(EmitterContext * ctx, ASTNode * node);
void emit_sub_assignment(EmitterContext * ctx, ASTNode * node);
void emit_binary_div(EmitterContext * ctx, ASTNode * node);
void emit_binary_mod(EmitterContext * ctx, ASTNode * node);
void emit_unary(EmitterContext * ctx, ASTNode * node);
void emit_if_statement(EmitterContext * ctx, ASTNode * node);
void emit_while_statement(EmitterContext * ctx, ASTNode * node);
void emit_do_while_statement(EmitterContext * ctx, ASTNode * node);
void emit_block(EmitterContext * ctx, ASTNode * node, bool enterNewScope);
void emit_function(EmitterContext * ctx, ASTNode * node);
void emit_var_declaration(EmitterContext * ctx, ASTNode * node);
void emit_assignment(EmitterContext * ctx, ASTNode* node);
void emit_add_assignment(EmitterContext * ctx, ASTNode * node);
void emit_sub_assignment(EmitterContext * ctx, ASTNode * node);
void emit_for_statement(EmitterContext * ctx, ASTNode * node);
void emit_pass_argument(EmitterContext * ctx, CType * type, ASTNode * node);
void emit_function_call(EmitterContext * ctx, ASTNode * node);
void emit_switch_dispatch(EmitterContext * ctx, ASTNode * node);
void emit_switch_bodies(EmitterContext * ctx, ASTNode * node);
void emit_switch_statement(EmitterContext * ctx, ASTNode * node);
void emit_case_statement(EmitterContext * ctx, ASTNode * node);
void emit_break_statement(EmitterContext * ctx, ASTNode * node);
void emit_continue_statement(EmitterContext * ctx, ASTNode * node);
void emit_csst(EmitterContext * ctx, ASTNode * node);
void emit_binary_multi(EmitterContext * ctx, ASTNode * node);
void emit_binary_sub(EmitterContext * ctx, ASTNode * node);
void emit_pointer_arithmetic(EmitterContext * ctx, CType * c_type);

void reset_size_and_offsets();
#endif