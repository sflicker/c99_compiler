#pragma once

#include "ast.h"
#include "list_util.h"

typedef enum {
    IR_CONST,
    IR_BINOP,
    IR_MOV,
    IR_LABEL,
    IR_JMP,
    IR_BR_IF_FALSE,
    IR_RETURN,
    IR_CALL,
    IR_ARG,
    IR_ADD,
    IR_MUL,
    IR_PARAM,
} IROp;

typedef struct IRInstr {
    IROp op;
    char * dst;
    char * src1;
    char * src2;
    char * label;
    char * binop;
} IRInstr;

DEFINE_LINKED_LIST(IRInstr*, IR_list);

typedef struct {
    char * name;
    IR_list * body;
    int param_count;
} IRFunction;

DEFINE_LINKED_LIST(IRFunction*, IRFunction_list);

typedef struct {
    IRFunction_list * functions;
} IRProgram;

void ir_append(IR_list * list, IRInstr * instr);
void ir_print(const IR_list * list);
void ir_free(IRInstr * ir);
void ir_function_list_free(IRFunction * function);
