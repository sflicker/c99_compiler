#include "ast.h"
#include "list_util.h"
#include "ir.h"
#include "ir_gen.h"

int temp_count = 0;
int label_count = 0;

char * new_temp() {
    char * buf = malloc(16);
    sprintf(buf, "L%d", temp_count++);
    return buf;
}

char * new_label() {
    char * buf = malloc(16);
    sprintf(buf, "L%d", label_count++);
    return buf;
}

IRProgram * gen_ir_program(ASTNode * node) {
    if (node->type == AST_TRANSLATION_UNIT) {
        IRProgram * ir_program = malloc(sizeof(IRProgram));
        ir_program->functions = malloc(sizeof(IRFunction_list));
        IRFunction_list_init(ir_program->functions, ir_function_list_free);
        for (ASTNode_list_node * n = node->translation_unit.functions->head;n;n=n->next) {
            IRFunction * ir_function = gen_ir_function(n->value);
            IRFunction_list_append(ir_program->functions, ir_function);
        }
        return ir_program;
    }
    return NULL;
}

IRFunction * gen_ir_function(ASTNode * node) {
    if (node->type == AST_FUNCTION_DECL && !node->function_decl.declaration_only) {
        IRFunction * ir_function = malloc(sizeof(IRFunction));
        ir_function->name = strdup(node->function_decl.name);
        IR_list * irlist = malloc(sizeof(IR_list));
        IR_list_init(irlist, ir_free);
        ASTNode * body = node->function_decl.body;
        for (ASTNode_list_node * n = body->block.statements->head;n;n=n->next) {
            char * t = gen_ir_statement(irlist, n->value);
        }
        ir_function->body = irlist;
        return ir_function;
    }
    return NULL;
}

void gen_ir_statement(IR_list * list, ASTNode * node) {
    switch(node->type) {
        
        case AST_RETURN_STMT:
        {
            gen_ir_expression(node->return_stmt.expr);
            break;
        }
    }
}

char * gen_ir_expression(IR_list * list, ASTNode * node) {

}
