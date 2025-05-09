#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "emitter.h"
#include "token.h"
#include "symtab.h"

void emit_tree_node(FILE * out, ASTNode * node);

static int label_id = 0;

void emit_header(FILE* out) {
    fprintf(out, "section .text\n");
    fprintf(out, "global main\n");
    fprintf(out, "\n");

}

void emit_label(FILE * out, const char * prefix, int num) {
    fprintf(out, ".L%s%d:\n", prefix, num);
}

void emit_jump(FILE * out, const char * op, const char * prefix, int num) {
    fprintf(out, "%s .L%s%d\n", op, prefix, num);
}

void emit_binary_comparison(FILE * out, ASTNode * node) {
    // eval left-hand side -> result in eax -> push results onto the stack
    emit_tree_node(out, node->binary.lhs);
    fprintf(out, "push rax\n");

    // eval right-hand side -> reult in eax

    emit_tree_node(out, node->binary.rhs);

    // restore lhs into rcx
    fprintf(out, "pop rcx\n");
    fprintf(out, "mov ecx, ecx\n");   // zero upper bits

    // compare rcx (lhs) with eax (rhs), cmp rcx, eax means rcx - eax
    fprintf(out, "cmp ecx, eax\n");

    // emit proper setX based on operator type
    switch (node->type) {
        case AST_EQUAL:
            fprintf(out, "sete al\n");
            break;

        case AST_NOT_EQUAL:
            fprintf(out, "setne al\n");
            break;

        case AST_LESS_THAN:
            fprintf(out, "setl al\n");
            break;

        case AST_LESS_EQUAL:
            fprintf(out, "setle al\n");
            break;

        case AST_GREATER_THAN:
            fprintf(out, "setg al\n");
            break;

        case AST_GREATER_EQUAL:
            fprintf(out, "setge al\n");
            break;

        default:
            fprintf(stderr, "Unsupported comparison type in codegen.\n");
            exit(1);

    }

    // zero-extend result to full eax
    fprintf(out, "movzx eax, al\n");

}


void emit_binary_op(FILE * out, ASTNodeType op) {
    switch(op) {
        case AST_ADD:
            fprintf(out, "add eax, ecx\n");
            break;
        case AST_SUB:
            fprintf(out, "sub eax, ecx\n");
            break;
        case AST_MUL:
            fprintf(out, "imul eax, ecx\n");
            break;
        // case TOKEN_DIV:
        //     fprintf(out, "cdq\n");          // sign-extend eax into edx:eax
        //     fprintf(out, "idiv ecx\n");
        //     break;
        // case TOKEN_EQ:
        //     fprintf(out, "sete al\n");
        //     break;
        // case TOKEN_NEQ:
        //     fprintf(out, "setne al\n");
        //     break;
        // case TOKEN_GT:
        //     fprintf(out, "setg al\n");
        //     break;
        // case TOKEN_GE:
        //     fprintf(out, "setge al\n");
        //     break;
        // case TOKEN_LT:
        //     fprintf(out, "setl al\n");
        //     break;
        // case TOKEN_LE:
        //     fprintf(out, "setle al\n");
        //     break;
        default:
            fprintf(stderr, "Unsupported binary operator: %s\n", token_type_name(op));
    }
}

void emit_unary(FILE *out, ASTNode * node) {
    switch (node->type) {
        case AST_UNARY_NEGATE:
            emit_tree_node(out, node->unary.operand);
            fprintf(out, "neg eax\n");
            break;
        case AST_UNARY_PLUS:
            // noop
            break;
        case AST_UNARY_NOT:
            // !x becomes (x == 0) -> 1 else 0
            emit_tree_node(out, node->unary.operand);
            fprintf(out, "cmp eax, 0\n");
            fprintf(out, "sete al\n");
            fprintf(out, "movzx eax, al\n");
            break;
        case AST_UNARY_PRE_INC: {
            int offset = lookup_symbol(node->unary.operand->var_expression.name);
            fprintf(out, "mov eax, [rbp%d]\n", offset);            
            fprintf(out, "add eax, 1\n");
            fprintf(out, "mov [rbp%d], eax\n", offset);
            break;
        }
        case AST_UNARY_PRE_DEC: {
            int offset = lookup_symbol(node->unary.operand->var_expression.name);
            fprintf(out, "mov eax, [rbp%d]\n", offset);            
            fprintf(out, "sub eax, 1\n");
            fprintf(out, "mov [rbp%d], eax\n", offset);
        break;
        }
        case AST_UNARY_POST_INC: {
            int offset = lookup_symbol(node->unary.operand->var_expression.name);
            fprintf(out, "mov eax, [rbp%d]\n", offset);
            fprintf(out, "mov ecx, eax\n");
            fprintf(out, "add eax, 1\n");
            fprintf(out, "mov [rbp%d], eax\n", offset);
            fprintf(out, "mov eax, ecx\n");
            break;
        }
        case AST_UNARY_POST_DEC: {
            int offset = lookup_symbol(node->unary.operand->var_expression.name);
            fprintf(out, "mov eax, [rbp%d]\n", offset);
            fprintf(out, "mov ecx, eax\n");
            fprintf(out, "sub eax, 1\n");
            fprintf(out, "mov [rbp%d], eax\n", offset);
            fprintf(out, "mov eax, ecx\n");
            break;
        }
        default:
            fprintf(stderr, "Unsupported unary op in emitter\n");
            exit(1);
    }
}

void emit_if_statement(FILE * out, ASTNode * node) {
    int id = label_id++;
    // eval condition
    emit_tree_node(out, node->if_stmt.cond);
    // compare result with 0
    fprintf(out, "cmp eax, 0\n");

    if (node->if_stmt.else_statement) {
        fprintf(out, "je .Lelse%d\n", id);  // jump to else if false
        emit_tree_node(out, node->if_stmt.then_statement);
        fprintf(out, "jmp .Lend%d\n", id);  // jump to end over else
        emit_label(out, "else", id);
//        fprintf(out, ".Lelse%d:\n", id);
        emit_tree_node(out, node->if_stmt.else_statement);
    }
    else {
        fprintf(out, "je .Lend%d\n", id);  // skip over if false
        emit_tree_node(out, node->if_stmt.then_statement);
    }
    emit_label(out, "end", id);
//    fprintf(out, ".Lend%d:\n", id);
}

void emit_while_statement(FILE* out, ASTNode * node) {
    int id = label_id++;
    // start label
    emit_label(out, "while_start", id);
    // eval cond
    emit_tree_node(out, node->while_stmt.cond);
    // cmp to zero
    fprintf(out, "cmp eax, 0\n");
    // jmp to end if condition not met
    fprintf(out, "je .Lwhile_end%d\n", id);
    emit_tree_node(out, node->while_stmt.body);
    // jmp to start
    fprintf(out, "jmp .Lwhile_start%d\n", id);
    emit_label(out, "while_end", id); 
}

void populate_symbol_table(ASTNode * node) {
    if (!node) {
        return;
    }

    switch(node->type) {
        case AST_PROGRAM:
            populate_symbol_table(node->program.function);
            break;
        case AST_FUNCTION:
            populate_symbol_table(node->function.body);
            break;
        
        case AST_BLOCK:
            for (int i=0;i<node->block.count;i++) {
                populate_symbol_table(node->block.statements[i]);
            }
            break;
        case AST_VAR_DECL:
            add_symbol(node->declaration.name);
            if (node->declaration.init_expr) {
                populate_symbol_table(node->declaration.init_expr);
            }
            break;
        case AST_ASSIGNMENT:
        case AST_COMPOUND_ADD_ASSIGN:
        case AST_COMPOUND_SUB_ASSIGN:
            add_symbol(node->assignment.name);
            populate_symbol_table(node->assignment.expr);
            break;
        case AST_RETURN_STMT:
            populate_symbol_table(node->return_stmt.expr);
            break;

        case AST_IF_STMT:
            populate_symbol_table(node->if_stmt.cond);
            populate_symbol_table(node->if_stmt.then_statement);
            if (node->if_stmt.else_statement) {
                populate_symbol_table(node->if_stmt.else_statement);
            }
            break;

        case AST_EXPRESSION_STMT:
            populate_symbol_table(node->expr_stmt.expr);
            break;

        // case AST_BINARY:
        //     populate_symbol_table(node->binary_op.lhs);
        //     populate_symbol_table(node->binary_op.rhs);
        //     break;

        case AST_UNARY_POST_INC:
        case AST_UNARY_POST_DEC:
        case AST_UNARY_PRE_INC:
        case AST_UNARY_PRE_DEC:
        case AST_UNARY_NEGATE:
        case AST_UNARY_NOT:
        case AST_UNARY_PLUS:
            populate_symbol_table(node->unary.operand);
            break;
        case AST_VAR_EXPR:
            add_symbol(node->var_expression.name);
            break;

        default:
            break;
    }
}

void emit_function(FILE * out, ASTNode * node) {



    // create label for function
    fprintf(out, "%s:\n", node->function.name);

    fprintf(out, "push rbp\n");
    fprintf(out,  "mov rbp, rsp\n");

    int local_space = get_symbol_total_space();
    if (local_space > 0) {
        //local_space = 16; //TODO may need to round the original local space to a 16 boundary
        fprintf(out, "sub rsp, %d\n", local_space);
    }
    
    emit_tree_node(out, node->function.body);

//    fprintf(out, "pop rbp\n");
    fprintf(out, "leave\n");
    fprintf(out, "ret\n");

}

void emit_var_declaration(FILE *out, ASTNode * node) {
    int offset = add_symbol(node->declaration.name);
    if (node->declaration.init_expr) {
        emit_tree_node(out, node->declaration.init_expr);
        fprintf(out, "mov [rbp%d], eax\n", offset);
    }
}

void emit_assignment(FILE * out, ASTNode* node) {
    emit_tree_node(out, node->assignment.expr);
    int offset = lookup_symbol(node->assignment.name);
    fprintf(out, "mov [rbp%d], eax\n", offset);
}

void emit_add_assignment(FILE *out, ASTNode * node) {
    int offset = lookup_symbol(node->assignment.name);
    fprintf(out, "mov eax, [rbp%d]\n", offset);
    fprintf(out, "push rax\n");
    emit_tree_node(out, node->assignment.expr);
    fprintf(out, "pop rcx\n");
    fprintf(out, "add eax, ecx\n");
    fprintf(out, "mov [rbp%d], eax\n", offset);
}

void emit_sub_assignment(FILE *out, ASTNode * node) {
    int offset = lookup_symbol(node->assignment.name);
    emit_tree_node(out, node->assignment.expr);
    fprintf(out, "mov ecx, eax\n");
    fprintf(out, "mov eax, [rbp%d]\n", offset);
    fprintf(out, "sub eax, ecx\n");
    fprintf(out, "mov [rbp%d], eax\n", offset);
}

void emit_for_statement(FILE * out, ASTNode * node) {
    int label_start = label_id++;
    int label_cond = label_id++;
    int label_end = label_id++;

    if (node->for_stmt.init_expr) {
        emit_tree_node(out, node->for_stmt.init_expr);
    }

    emit_jump(out, "jmp", "cond", label_cond);

    emit_label(out, "start", label_start);
    emit_tree_node(out, node->for_stmt.body);
    emit_tree_node(out, node->for_stmt.update_expr);
    emit_label(out, "cond", label_cond);
    emit_tree_node(out, node->for_stmt.cond_expr);
    fprintf(out, "cmp eax, 0\n");

    emit_jump(out, "je", "end", label_end);
    emit_jump(out, "jmp", "start", label_start);
    emit_label(out, "end", label_end);
}


void emit_tree_node(FILE * out, ASTNode * node) {
    if (!node) return;
    switch(node->type) {
        case AST_PROGRAM:
            emit_header(out);
            emit_tree_node(out, node->program.function);
            break;
        case AST_FUNCTION:
            emit_function(out, node);
            break;
        case AST_VAR_DECL:
            emit_var_declaration(out, node);
            break;
        case AST_ASSIGNMENT: 
            emit_assignment(out, node);
            break;
        case AST_COMPOUND_ADD_ASSIGN:
            emit_add_assignment(out, node);
            break;
        case AST_COMPOUND_SUB_ASSIGN:
            emit_sub_assignment(out, node);
            break;
        case AST_RETURN_STMT:
            emit_tree_node(out, node->return_stmt.expr);
//            fprintf(out, "ret\n");
            break;
        case AST_EXPRESSION_STMT:
            emit_tree_node(out, node->expr_stmt.expr);
            break;
        case AST_BLOCK:
            for (int i=0;i<node->block.count;i++) {
                emit_tree_node(out, node->block.statements[i]);
            }
            break;
        case AST_IF_STMT:
            emit_if_statement(out, node);
            break;
        case AST_WHILE_STMT:
            emit_while_statement(out, node);
            break;
        case AST_DIV: {
            emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
            fprintf(out, "push rax\n");                     // push lhs result
            emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
            fprintf(out, "mov ecx, eax\n");                 // move denominator to ecx
            fprintf(out, "pop rax\n");                      // restore numerator to eax
            fprintf(out, "cdq\n");
            fprintf(out, "idiv ecx\n");
            break;
        }
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
            emit_tree_node(out, node->binary.lhs);       // codegen to eval lhs with result in EAX
            fprintf(out, "push rax\n");                     // push lhs result
            emit_tree_node(out, node->binary.rhs);       // codegen to eval rhs with result in EAX
            fprintf(out, "pop rcx\n");                      // pop lhs to ECX
            emit_binary_op(out, node->type);        // emit proper for op
        break;
            // else if (node->binary_op.op == TOKEN_EQ || node->binary_op.op == TOKEN_NEQ ||
            //         node->binary_op.op == TOKEN_LT || node->binary_op.op == TOKEN_LE ||
            //         node->binary_op.op == TOKEN_GT || node->binary_op.op == TOKEN_GE) {
            //             emit_tree_node(out, node->binary_op.lhs);
            //             fprintf(out, "push rax\n");
            //             emit_tree_node(out, node->binary_op.rhs);
            //             fprintf(out, "mov rcx, rax\n");
            //             fprintf(out, "pop rax\n");
            //             fprintf(out, "cmp eax, ecx\n");
            //             emit_binary_op(out, node->binary_op.op);
            //             fprintf(out, "movzx eax, al\n");
            //         }
            // break;

        case AST_EQUAL:
        case AST_NOT_EQUAL:
        case AST_LESS_EQUAL:
        case AST_LESS_THAN:
        case AST_GREATER_EQUAL:
        case AST_GREATER_THAN:
            emit_binary_comparison(out, node);
            break;

        case AST_UNARY_POST_INC:
        case AST_UNARY_POST_DEC:
        case AST_UNARY_PRE_INC:
        case AST_UNARY_PRE_DEC:
        case AST_UNARY_NEGATE:
        case AST_UNARY_NOT:
        case AST_UNARY_PLUS:
            emit_unary(out, node);
            break;

        case AST_INT_LITERAL:
            fprintf(out, "mov eax, %d\n", node->int_value);
            break;
        case AST_VAR_EXPR:
            int offset = lookup_symbol(node->var_expression.name);
            fprintf(out, "mov eax, [rbp%d]\n", offset);
            break;
        case AST_FOR_STMT:
            emit_for_statement(out, node);
            break;

    }
}

void emit_program(ASTNode * program, const char * output_file) {
    FILE * ptr = fopen(output_file, "w");
    init_symbol_table();
    populate_symbol_table(program);
    emit_tree_node(ptr, program);

    fclose(ptr);
}