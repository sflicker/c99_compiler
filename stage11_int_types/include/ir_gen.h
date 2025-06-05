#include "list_util.h"
#include "ast.h"
#include "ir.h"

IRProgram * gen_ir_program(ASTNode * node);
IRFunction * gen_ir_function(ASTNode * node);
IRInstr * gen_ir_statement(ASTNode * node);