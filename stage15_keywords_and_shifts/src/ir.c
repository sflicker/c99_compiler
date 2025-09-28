#include "ir.h"

void ir_append(IR_list * list, IRInstr * instr) {
    IRInstr * node = malloc(sizeof(IRInstr));
    IR_list_append(list, node);
}

void ir_function_list_free(IRFunction * function) {

}

void ir_free(IRInstr * ir) {
    
}