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

void emit_binary_op(FILE * out, TokenType op) {
    switch(op) {
        case TOKEN_PLUS:
            fprintf(out, "add eax, ecx\n");
            break;
        case TOKEN_MINUS:
            fprintf(out, "sub eax, ecx\n");
            break;
        case TOKEN_STAR:
            fprintf(out, "imul eax, ecx\n");
            break;
        case TOKEN_DIV:
            fprintf(out, "cdq\n");          // sign-extend eax into edx:eax
            fprintf(out, "idiv ecx\n");
            break;
        case TOKEN_EQ:
            fprintf(out, "sete al\n");
            break;
        case TOKEN_NEQ:
            fprintf(out, "setne al\n");
            break;
        case TOKEN_GT:
            fprintf(out, "setg al\n");
            break;
        case TOKEN_GE:
            fprintf(out, "setge al\n");
            break;
            case TOKEN_LT:
            fprintf(out, "setl al\n");
            break;
        case TOKEN_LE:
            fprintf(out, "setle al\n");
            break;
        default:
            fprintf(stderr, "Unsupported binary operator: %s\n", token_type_name(op));
    }
}

void emit_unary_op(FILE *out, TokenType op) {
    switch (op) {
        case TOKEN_MINUS:
            fprintf(out, "neg eax\n");
            break;
        case TOKEN_PLUS:
            // noop
            break;
        case TOKEN_BANG:
            // !x becomes (x == 0) -> 1 else 0
            fprintf(out, "cmp eax, 0\n");
            fprintf(out, "sete al\n");
            fprintf(out, "movzx eax, al\n");
            break;
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
        fprintf(out, ".Lelse%d\n", id);
        emit_tree_node(out, node->if_stmt.else_statement);
    }
    else {
        fprintf(out, "je .Lend%d\n", id);  // skip over if false
        emit_tree_node(out, node->if_stmt.then_statement);
    }
    fprintf(out, ".Lend%d:\n", id);
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

        case AST_BINARY_OP:
            populate_symbol_table(node->binary_op.lhs);
            populate_symbol_table(node->binary_op.rhs);
            break;

        case AST_UNARY_OP:
            populate_symbol_table(node->unary_op.expr);
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
        fprintf(out, "sub rsp, %d\n", local_space);
    }
    
    emit_tree_node(out, node->function.body);

    fprintf(out, "pop rbp\n");
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
        case AST_BINARY_OP:
            if (node->binary_op.op == TOKEN_DIV) {
                emit_tree_node(out, node->binary_op.lhs);       // codegen to eval lhs with result in EAX
                fprintf(out, "push rax\n");                     // push lhs result
                emit_tree_node(out, node->binary_op.rhs);       // codegen to eval rhs with result in EAX
                fprintf(out, "mov ecx, eax\n");                 // move denominator to ecx
                fprintf(out, "pop rax\n");                      // restore numerator to eax
                fprintf(out, "cdq\n");
                fprintf(out, "idiv ecx\n");
            }
            else if (node->binary_op.op == TOKEN_PLUS || node->binary_op.op == TOKEN_MINUS || node->binary_op.op == TOKEN_STAR) {
                emit_tree_node(out, node->binary_op.lhs);       // codegen to eval lhs with result in EAX
                fprintf(out, "push rax\n");                     // push lhs result
                emit_tree_node(out, node->binary_op.rhs);       // codegen to eval rhs with result in EAX
                fprintf(out, "pop rcx\n");                      // pop lhs to ECX
                emit_binary_op(out, node->binary_op.op);        // emit proper for op
            }
            else if (node->binary_op.op == TOKEN_EQ || node->binary_op.op == TOKEN_NEQ ||
                    node->binary_op.op == TOKEN_LT || node->binary_op.op == TOKEN_LE ||
                    node->binary_op.op == TOKEN_GT || node->binary_op.op == TOKEN_GE) {
                        emit_tree_node(out, node->binary_op.lhs);
                        fprintf(out, "push rax\n");
                        emit_tree_node(out, node->binary_op.rhs);
                        fprintf(out, "mov rcx, rax\n");
                        fprintf(out, "pop rax\n");
                        fprintf(out, "cmp eax, ecx\n");
                        emit_binary_op(out, node->binary_op.op);
                        fprintf(out, "movzx eax, al\n");
                    }
            break;
        case AST_UNARY_OP:
            emit_tree_node(out, node->unary_op.expr);  // generate value into eax
            emit_unary_op(out, node->unary_op.op);
            break;
        case AST_INT_LITERAL:
            fprintf(out, "mov eax, %d\n", node->int_value);
            break;
        case AST_VAR_EXPR:
            int offset = lookup_symbol(node->var_expression.name);
            fprintf(out, "mov eax, [rbp%d]\n", offset);
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