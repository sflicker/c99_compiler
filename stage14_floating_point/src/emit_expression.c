//
// Created by scott on 8/9/25.
//

#include "internal.h"
#include "ast.h"

#include "emitter.h"
#include "emitter_context.h"
#include "emitter_helpers.h"
#include "emit_address.h"
#include "emit_expression.h"

#include "error.h"

INTERNAL void emit_function_call_expr(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    // if the call has arguments
    // first get a reversed list
    // then emit each arg then push it
    //struct node_list * reversed_list = NULL;

    int arg_count=0;
    if (node->function_call.arg_list) {

        // loop through in reverse order pushing arguments to the stack
        for (int i = node->function_call.arg_list->count - 1; i >= 0; i--) {
            ASTNode * argNode = ASTNode_list_get(node->function_call.arg_list, i);
            emit_int_expr_to_rax(ctx, argNode, WANT_VALUE);
            //            emit_line(ctx, "push rax");
            arg_count++;
        }

        // for (ASTNode_list_node *n = node->function_call.arg_list->head;n;n=n->next) {
        //     ASTNode * argNode = n->value;
        //     emit_tree_node(ctx, argNode);
        //     emit_line(ctx, "push rax\n");
        //     // if (arg_count<ARG_REG_COUNT) {
        //     //     const char * reg = ARG_REGS[arg_count];
        //     //     emit_line(ctx, "mov %s, rax\n", reg);
        //     // }  //TODO SUPPORT MORE THAN 6 arguments using the stack
        // }

    }

    // // call the function
    emit_line(ctx, "call %s", node->function_call.name);

    // // clean up arguments
    if (arg_count > 0) {
        emit_add_rsp(ctx, arg_count*8);
    }

    //    emit_line(ctx, "push rax");
    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }

}


INTERNAL void emit_int_cast_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {

    if (is_floating_point_type(node->cast_expr.expr->ctype)) {
        emit_fp_expr_to_xmm0(ctx, node->cast_expr.expr, mode);
        if (mode == WANT_VALUE) {
            emit_fpop(ctx, "xmm0", FP32);
        }
        emit_line(ctx, "cvttsd2si, eax, xmm0");
        if (mode == WANT_VALUE) {
            emit_push(ctx, "rax");
        }
        return;
    }

    emit_int_expr_to_rax(ctx, node->cast_expr.expr, WANT_VALUE);     // eval inner expression

    CType * from_type = node->cast_expr.expr->ctype;
    CType * to_type = node->cast_expr.target_type;

    int from_size = from_type->size;
    int to_size = to_type->size;

    bool from_signed = from_type->is_signed;
    bool to_signed = to_type->is_signed;

    if ((from_type->kind == CTYPE_PTR || from_type->kind == CTYPE_ARRAY) &&
        (to_type->kind == CTYPE_PTR || to_type->kind == CTYPE_ARRAY)) {
        // pointer to pointer or array to pointer: no-op in codegen
        return;
    }



    if (from_size == to_size) {
        return;  // NOOP
    }

    // narrowing
    if (from_size > to_size) {
        //emit_line(ctx, "pop rax");
        emit_pop(ctx, "rax");
        switch (to_size) {
            case 1: emit_line(ctx, "movsx eax, al"); break;
            case 2: emit_line(ctx, "movsx eax, ax"); break;
            case 4: emit_line(ctx, "mov eax, eax"); break;
            default:
                error("Unsupported narrowing cast to %d bytes", to_size);
        }
        if (mode == WANT_VALUE) {
            emit_push(ctx, "rax");
        }
        return;
    }

    // widening cast
    if (from_size < to_size) {
        emit_pop(ctx, "rax");

        if (from_signed && to_signed) {
            switch (from_size) {
                case 1: emit_line(ctx, "movsx eax, al"); break;
                case 2: emit_line(ctx, "movsx eax, ax"); break;
                case 4: emit_line(ctx, "movsxd rax, eax"); break;
                default:
                    error("Unsupported sign-extension from %d bytes", from_size);
            }
        } else {
            switch (from_size) {
                case 1: emit_line(ctx, "movzx eax, al"); break;
                case 2: emit_line(ctx, "movzx eax, ax"); break;
                case 4: emit_line(ctx, "mov eax, eax"); break;
                default:
                    error("Unsupported zero-extension from %d bytes", from_size);
            }
        }
        if (mode == WANT_VALUE) {
            emit_push(ctx, "rax");
        }
    }
}

INTERNAL void emit_int_assignment_expr_to_rax(EmitterContext * ctx, ASTNode* node, EvalMode mode) {
    emit_line(ctx, "; emitting assignment - LHS %s = RHS %s",
        get_ast_node_name(node->binary.lhs), get_ast_node_name(node->binary.rhs));
    // eval RHS -> rax then push
    if (is_array_type(node->binary.rhs->ctype)) {
        emit_addr_to_rax(ctx, node->binary.rhs);
    }
    else {
        emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);
    }
    //    emit_line(ctx, "push rax");

    // eval LHS addr -> rcx
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_ADDRESS);

    // pop LHS into rcx
    //    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");

    // pop RHS in rax
    //    emit_line(ctx, "pop rax");
    emit_pop(ctx, "rax");

    CType * ctype = node->binary.lhs->ctype;

    emit_line(ctx, "mov %s [rcx], %s",
        mem_size_for_type(ctype),
        reg_for_type(ctype));

}

INTERNAL void emit_int_add_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);       // codegen to eval lhs with result in EAX
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);       // codegen to eval rhs with result in EAX

//    emit_line(ctx, "pop rcx");                      // pop rhs to RCX
//    emit_line(ctx, "pop rax");                      // pop lhs to RAX
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");


    CType *lhs_type = node->binary.lhs->ctype;
    CType *rhs_type = node->binary.rhs->ctype;

    if (lhs_type->kind == CTYPE_PTR && is_integer_type(rhs_type)) {
        int elem_size = sizeof_type(lhs_type->base_type);
        emit_line(ctx, "imul rcx, %d", elem_size);
        emit_line(ctx, "add rax, rcx");
    }
    else if (is_integer_type(lhs_type) && rhs_type->kind == CTYPE_PTR) {
        int elem_size = sizeof_type(rhs_type->base_type);
        emit_line(ctx, "imul rax, %d", elem_size);
        emit_line(ctx, "add rax, rcx");
    }
    else if (is_integer_type(lhs_type) && is_integer_type(rhs_type)) {
        emit_line(ctx, "; lhs in rax, rhs in rcx");
        if (lhs_type->kind == CTYPE_CHAR) {
            emit_line(ctx, "movsx rax, al");
        } else if (lhs_type->kind == CTYPE_SHORT) {
            emit_line(ctx, "movsx rax, ax");
        } else if (lhs_type->kind == CTYPE_INT) {
            emit_line(ctx, "movsxd rax, eax");
        }

        if (rhs_type->kind == CTYPE_CHAR) {
            emit_line(ctx, "movsx rcx, cl");
        } else if (rhs_type->kind == CTYPE_SHORT) {
            emit_line(ctx, "movsx rcx, cx");
        } else if (rhs_type->kind == CTYPE_INT) {
            emit_line(ctx, "movsxd rcx, ecx");
        }

        emit_line(ctx, "add rax, rcx");
    }
    else {
        error("Unsupported types for binary add operation");
    }
//    emit_binary_op(ctx, node->binary.op);        // emit proper for op

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
//    emit_line(ctx, "push rax");

}

INTERNAL void emit_int_sub_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);       // codegen to eval lhs with result in EAX
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);       // codegen to eval rhs with result in EAX

    // emit_line(ctx, "pop rcx");                      // pop rhs to RCX
    // emit_line(ctx, "pop rax");                      // pop lhs to RAX
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");


    CType *lhs_type = node->binary.lhs->ctype;
    CType *rhs_type = node->binary.rhs->ctype;

    if (lhs_type->kind == CTYPE_PTR && is_integer_type(rhs_type)) {
        int elem_size = sizeof_type(lhs_type);
        emit_line(ctx, "imul rcx, %d", elem_size);
        emit_line(ctx, "add rax, rcx");
    }
    else if (is_integer_type(lhs_type) && rhs_type->kind == CTYPE_PTR) {
        int elem_size = sizeof_type(rhs_type);
        emit_line(ctx, "imul rax, %d", elem_size);
        emit_line(ctx, "add rax, rcx");
    }
    else if (is_integer_type(lhs_type) && is_integer_type(rhs_type)) {
        emit_line(ctx, "; lhs in rax, rhs in rcx");
        if (lhs_type->kind == CTYPE_CHAR) {
            emit_line(ctx, "movsx rax, al");
        } else if (lhs_type->kind == CTYPE_SHORT) {
            emit_line(ctx, "movsx rax, ax");
        } else if (lhs_type->kind == CTYPE_INT) {
            emit_line(ctx, "movsxd rax, eax");
        }

        if (rhs_type->kind == CTYPE_CHAR) {
            emit_line(ctx, "movsx rcx, cl");
        } else if (rhs_type->kind == CTYPE_SHORT) {
            emit_line(ctx, "movsx rcx, cx");
        } else if (rhs_type->kind == CTYPE_INT) {
            emit_line(ctx, "movsxd rcx, ecx");
        }

        emit_line(ctx, "sub rax, rcx");
    }
    else {
        error("Unsupported types for binary add operation");
    }
    //    emit_binary_op(ctx, node->binary.op);        // emit proper for op

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
//    emit_line(ctx, "push rax");

}


INTERNAL void emit_int_multi_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);       // codegen to eval lhs with result in EAX
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);       // codegen to eval rhs with result in EAX

    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");

    emit_binary_op(ctx, node->binary.op);        // emit proper for op

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
}

void emit_int_div_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);       // codegen to eval lhs with result in EAX
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);       // codegen to eval rhs with result in EAX
    // emit_line(ctx, "pop rcx");           // pop rhs to rCX
    // emit_line(ctx, "pop rax");           // pop lhs to rax
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");


    // emit_line(ctx, "mov ecx, eax");                 // move denominator to ecx
    // emit_line(ctx, "pop rax");                      // restore numerator to eax
    emit_line(ctx, "cdq");
    emit_line(ctx, "idiv ecx");
    //    emit_line(ctx, "push rax");
    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }


}

INTERNAL void emit_int_mod_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {

    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);       // codegen to eval lhs with result in EAX
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);       // codegen to eval rhs with result in EAX
    // emit_line(ctx, "pop rcx");           // pop rhs to rCX
    // emit_line(ctx, "pop rax");           // pop lhs to rax
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");

    emit_line(ctx, "cdq");
    emit_line(ctx, "idiv ecx");               // divide eax by ecx. result goes to eax, remainder to edx
    emit_line(ctx, "mov eax, edx");           // move remainer in edx to eax

    //    emit_line(ctx, "push rax");
    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }

}

INTERNAL void emit_int_logical_and_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    int label_false = get_label_id(ctx);
    int label_end = get_label_id(ctx);

    //lhs
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);
    emit_pop(ctx, "rax");
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "je", "false", label_false);

    //rhs
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);
    emit_pop(ctx, "rax");
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "je", "false", label_false);

    // both true
    emit_line(ctx, "mov eax, 1");
    emit_jump(ctx, "jmp", "end", label_end);

    emit_label(ctx, "false", label_false);
    emit_line(ctx, "mov eax, 0");

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }

    emit_label(ctx, "end", label_end);
}

INTERNAL void emit_int_logical_or_expr_to_rax(EmitterContext * ctx, ASTNode* node, EvalMode mode) {
    int label_true = get_label_id(ctx);
    int label_end = get_label_id(ctx);

    // lhs
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);
    emit_pop(ctx, "rax");
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "jne", "true", label_true);

    // rhs
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);
    emit_pop(ctx, "rax");
    emit_line(ctx, "cmp eax, 0");
    emit_jump(ctx, "jne", "true", label_true);

    emit_label(ctx, "true", label_true);
    emit_line(ctx, "mov eax, 1");

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }

    emit_label(ctx, "end", label_end);
}

INTERNAL void emit_int_add_assignment_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {

    // compute lhs address -> rcx
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_ADDRESS);

    emit_pop(ctx, "rcx");               // pop rcx off the stack to load the lhs value
    emit_push(ctx, "rcx");              // push rcx back on the stack to latter store the result
    // load lhs into rax
    emit_line(ctx, "mov DWORD eax, [rcx]");
//    emit_line(ctx, "push rax");
    emit_push(ctx, "rax");


    // evaluate RHS into eax
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);

    // restore RHS into ecx
//    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");

    // restore LHS into eax
//    emit_line(ctx, "pop rax");
    emit_pop(ctx, "rax");


    // add RHS to LHS
    emit_line(ctx, "add eax, ecx");

    // restore LHS address
//    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");


    // write back result to LHS
    emit_line(ctx, "mov [rcx], eax");

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
}

INTERNAL void emit_int_sub_assignment_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {

    // compute lhs address -> rcx
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_ADDRESS);

    emit_pop(ctx, "rcx");               // pop rcx off the stack to load the lhs value
    emit_push(ctx, "rcx");              // push rcx back on the stack to latter store the result

    // push LHS address on stack
    emit_line(ctx, "mov DWORD eax, [rcx]");
//    emit_line(ctx, "push rax");
    emit_push(ctx, "rax");


    // // load current value into eax
    // emit_line(ctx, "mov eax, [rcx]");
    //
    // // save current RHS value in eax on stack
    // emit_line(ctx, "push rax");

    // evaluate RHS into eax
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);

//    emit_line(ctx, "mov ecx, eax");

    // restore RHS into ecx
//    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");

    // restore LHS into eax
//    emit_line(ctx, "pop rax");
    emit_pop(ctx, "rax");

    // sub RHS from LHS
    emit_line(ctx, "sub eax, ecx");

    // restore LHS address
//    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");

    // write back result to LHS
    emit_line(ctx, "mov [rcx], eax");

    // char * reference_label  = create_variable_reference(ctx, node->binary.lhs);
    // emit_tree_node(ctx, node->binary.rhs);
    // emit_line(ctx, "mov ecx, eax\n");
    // emit_line(ctx, "mov eax, %s\n", reference_label);
    // emit_line(ctx, "sub eax, ecx\n");
    // emit_line(ctx, "mov %s, eax\n", reference_label);
    // free(reference_label);

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
}

INTERNAL void emit_int_comparison_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    // eval left-hand side -> result in eax -> push results onto the stack
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);

    // emit_line(ctx, "pop rcx");
    // emit_line(ctx, "pop rax");
    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");

    // eval right-hand side -> reult in eax


    // restore lhs into rcx
    emit_line(ctx, "mov ecx, ecx");   // zero upper bits
    emit_line(ctx, "mov eax, eax");   // zero upper bits

    // compare rax (lhs) with ecx (rhs), cmp rcx, eax means rcx - eax
    emit_line(ctx, "cmp eax, ecx");

    // emit proper setX based on operator type
    switch (node->binary.op) {
        case BINOP_EQ:
            emit_line(ctx, "sete al");
            break;

        case BINOP_NE:
            emit_line(ctx, "setne al");
            break;

        case BINOP_LT:
            emit_line(ctx, "setl al");
            break;

        case BINOP_LE:
            emit_line(ctx, "setle al");
            break;

        case BINOP_GT:
            emit_line(ctx, "setg al");
            break;

        case BINOP_GE:
            emit_line(ctx, "setge al");
            break;

        default:
            error("Unsupported comparison type in codegen.");
    }

    // zero-extend result to full eax
    emit_line(ctx, "movzx eax, al");
    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
}


INTERNAL void emit_int_binary_expr_to_rax(EmitterContext * ctx, ASTNode *node, EvalMode mode) {
    switch (node->binary.op) {
        case BINOP_EQ:
        case BINOP_NE:
        case BINOP_GT:
        case BINOP_GE:
        case BINOP_LT:
        case BINOP_LE:
            emit_int_comparison_expr_to_rax(ctx, node, mode);
            break;
        case BINOP_ADD: {
            if (is_integer_type(node->ctype)) {
                emit_int_add_expr_to_rax(ctx, node, mode);
            }
        }
            break;
        case BINOP_SUB:
            emit_int_sub_expr_to_rax(ctx, node, mode);
            break;
        case BINOP_MUL:
            emit_int_multi_expr_to_rax(ctx, node, mode);
            //          emit_expr(ctx, node->binary.lhs);       // codegen to eval lhs with result in EAX
            // //         emit_line(ctx, "push rax");                     // push lhs result
            //          emit_expr(ctx, node->binary.rhs);       // codegen to eval rhs with result in EAX
            //          emit_line(ctx, "pop rcx");                      // pop lhs to ECX
            //          emit_line(ctx, "pop rax");
            //          emit_binary_op(ctx, node->binary.op);        // emit proper for op
            break;
        case BINOP_DIV:
            emit_int_div_expr_to_rax(ctx, node, mode);
            break;
        case BINOP_MOD:
            emit_int_mod_expr_to_rax(ctx, node, mode);
            break;
        case BINOP_LOGICAL_AND:
            emit_int_logical_and_expr_to_rax(ctx, node, mode);
            break;

        case BINOP_LOGICAL_OR:
            emit_int_logical_or_expr_to_rax(ctx, node, mode);
            break;
        case BINOP_ASSIGNMENT:
            emit_int_assignment_expr_to_rax(ctx, node, mode);
            break;
        case BINOP_COMPOUND_ADD_ASSIGN:
            emit_int_add_assignment_expr_to_rax(ctx, node, mode);
            break;
        case BINOP_COMPOUND_SUB_ASSIGN:
            emit_int_sub_assignment_expr_to_rax(ctx, node, mode);
            break;
        default:
            error("Unknown binary operator");
    }
}





INTERNAL void emit_unary(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    switch (node->unary.op) {
        case UNARY_NEGATE:
            emit_int_expr_to_rax(ctx, node->unary.operand, WANT_VALUE);
//            emit_line(ctx, "pop rax");
            emit_pop(ctx, "rax");

            emit_line(ctx, "neg eax");
//            emit_line(ctx, "push rax");
            if (mode == WANT_VALUE) {
                emit_push(ctx, "rax");
            }

            break;
        case UNARY_PLUS:
            // noop
            break;
        case UNARY_NOT:
            // !x becomes (x == 0) -> 1 else 0
            emit_int_expr_to_rax(ctx, node->unary.operand, WANT_VALUE);
            emit_pop(ctx, "rax");
            emit_line(ctx, "cmp eax, 0");
            emit_line(ctx, "sete al");
            emit_line(ctx, "movzx eax, al");
            if (mode == WANT_VALUE) {
                emit_push(ctx, "rax");
            }
            break;
        case UNARY_PRE_INC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s", reference_label);
            emit_line(ctx, "add eax, 1");
            emit_line(ctx, "mov %s, eax", reference_label);
            free(reference_label);
            break;
        }
        case UNARY_PRE_DEC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s", reference_label);
            emit_line(ctx, "sub eax, 1");
            emit_line(ctx, "mov %s, eax", reference_label);
            free(reference_label);
        break;
        }
        case UNARY_POST_INC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s", reference_label);
            emit_line(ctx, "mov ecx, eax");
            emit_line(ctx, "add eax, 1");
            emit_line(ctx, "mov %s, eax", reference_label);
            emit_line(ctx, "mov eax, ecx");
            free(reference_label);
            break;
        }
        case UNARY_POST_DEC: {
            char * reference_label = create_variable_reference(ctx, node->unary.operand);
            emit_line(ctx, "mov eax, %s", reference_label);
            emit_line(ctx, "mov ecx, eax");
            emit_line(ctx, "sub eax, 1");
            emit_line(ctx, "mov %s, eax", reference_label);
            emit_line(ctx, "mov eax, ecx");
            free(reference_label);
            break;
        }
        case UNARY_ADDRESS: {
            emit_int_expr_to_rax(ctx, node->unary.operand, WANT_ADDRESS);
//             char * reference_label = create_variable_reference(ctx, node->unary.operand);
//             emit_line(ctx, "lea rax, %s", reference_label);
// //            emit_line(ctx, "push rax");
//             emit_push(ctx, "rax");
//
//             free(reference_label);
            break;
        }
        case UNARY_DEREF: {
            emit_int_expr_to_rax(ctx, node->unary.operand, WANT_VALUE);
//            emit_line(ctx, "pop rax");
            emit_pop(ctx, "rax");

            if (node->ctype->kind == CTYPE_CHAR) {
                emit_line(ctx, "movzx eax, BYTE [rax]");
            } else if (node->ctype->kind == CTYPE_SHORT) {
                emit_line(ctx, "movzx eax, WORD [rax]");
            } else if (node->ctype->kind == CTYPE_INT) {
                emit_line(ctx, "mov eax, [rax]");
            }
            else if (node->ctype->kind == CTYPE_LONG) {
                emit_line(ctx, "mov rax, [rax]");
            }
//            emit_line(ctx, "push rax");
            if (mode == WANT_VALUE) {
                emit_push(ctx, "rax");
            }

            break;
        }
        default:
            error("Unsupported unary op in emitter");
    }
}

void emit_fp_add_expr_to_xmm0(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_fp_expr_to_xmm0(ctx, node->binary.lhs, WANT_VALUE);       // codegen to eval lhs with result in EAX
    emit_fp_expr_to_xmm0(ctx, node->binary.rhs, WANT_VALUE);

    emit_fpop(ctx, "xmm0", FP32);  //TODO fix so either 32 or 64 is used
    emit_fpop(ctx, "xmm1", FP32);

    emit_line(ctx, "addss xmm0, xmm1");

    emit_fpush(ctx, "xmm0", FP32);
}

void emit_fp_binary_expr_to_xmm0(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    switch (node->binary.op) {
        // case BINOP_EQ:
        // case BINOP_NE:
        // case BINOP_GT:
        // case BINOP_GE:
        // case BINOP_LT:
        // case BINOP_LE:
        //     emit_int_comparison_expr_to_rax(ctx, node, mode);
        //     break;
        case BINOP_ADD: {
            if (is_floating_point_type(node->ctype)) {
                emit_fp_add_expr_to_xmm0(ctx, node, mode);
            }
            break;
        }
        default:
            error("Unexpected node type %d", get_ast_node_name(node));
    }

}

void emit_fp_expr_to_xmm0(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    if (mode == WANT_ADDRESS) {
        emit_addr_to_rax(ctx, node);
        return;
    }
    switch (node->type) {
        case AST_FLOAT_LITERAL:
            emit_line(ctx, "movss xmm0, [rel %s]", node->float_literal.label);  // need label
            if (mode == WANT_VALUE) {
                emit_fpush(ctx, "xmm0", FP32);
            }
            break;

        case AST_DOUBLE_LITERAL:
            emit_line(ctx, "movsd xmm0, [rel %s]", node->double_literal.label);  // need label
            if (mode == WANT_VALUE) {
                emit_fpush(ctx, "xmm0", FP64);
            }
            break;

        case AST_BINARY_EXPR:
            emit_fp_binary_expr_to_xmm0(ctx, node, mode);
            break;

        case AST_VAR_REF_EXPR:
            emit_addr_to_rax(ctx, node);
            if (!is_array_type(node->ctype)) {
                if (is_floating_point_type(node->ctype)) {
                    //emit_fpop(ctx, "xmm0", FP32);
                    emit_pop(ctx, "rax");
                    emit_load_from(ctx, node->ctype, "rax");
                    if (mode == WANT_VALUE) {
                        emit_fpush(ctx, "xmm0", FP32);
                    }
                } else {
                    emit_pop(ctx, "rax");
                    emit_load_from(ctx, node->ctype, "rax");
                    if (mode == WANT_VALUE) {
                        emit_push(ctx, "rax");
                    }
                }
            }
            break;

        default:
            error("Unexpected node type %d", get_ast_node_name(node));
    }
}

// emit_expr.
// generate code to eval the expression storing the final result in eax or rax
void emit_int_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    if (mode == WANT_ADDRESS) {
        emit_addr_to_rax(ctx, node);
        return;
    }
    switch (node->type) {
        case AST_INT_LITERAL:
            emit_line(ctx, "mov eax, %d", node->int_value);
            if (mode == WANT_VALUE) {
                emit_push(ctx, "rax");
            }
            break;
        case AST_FLOAT_LITERAL:
            emit_line(ctx, "movss xmm0, [rel %s]", node->float_literal.label);  // need label
            if (mode == WANT_VALUE) {
                emit_fpush(ctx, "xmm0", FP32);
            }
            break;

        case AST_DOUBLE_LITERAL:
            emit_line(ctx, "movsd xmm0, [rel %s]", node->double_literal.label);  // need label
            if (mode == WANT_VALUE) {
                emit_fpush(ctx, "xmm0", FP64);
            }
            break;

        case AST_VAR_REF_EXPR: {
            emit_addr_to_rax(ctx, node);
            if (!is_array_type(node->ctype)) {
                if (is_floating_point_type(node->ctype)) {
                    //emit_fpop(ctx, "xmm0", FP32);
                    emit_pop(ctx, "rax");
                    emit_load_from(ctx, node->ctype, "rax");
                    if (mode == WANT_VALUE) {
                        emit_push(ctx, "xmm0");
                    }
                } else {
                    emit_pop(ctx, "rax");
                    emit_load_from(ctx, node->ctype, "rax");
                    if (mode == WANT_VALUE) {
                        emit_push(ctx, "rax");
                    }
                }
            }
            // if (node->symbol->ctype->kind == CTYPE_ARRAY) {
            //     emit_line(ctx, "lea rax, [rbp-%d]", abs(node->symbol->info.var.offset));
            //     emit_push(ctx, "rax");
            // }
            // else if (node->symbol->node->var_decl.is_global) {
            //     emit_line(ctx, "mov %s, [rel %s] ", reg_for_type(node->ctype), node->symbol->name);
            //     emit_push(ctx, "rax");
            // } else {
            //     int offset = node->symbol->info.var.offset;
            //     emit_line(ctx, "mov %s, [rbp%+d]", reg_for_type(node->ctype), offset);
            //     emit_push(ctx, "rax");
            // }
            break;
        }
        case AST_UNARY_EXPR:
            emit_unary(ctx, node, mode);
            break;
        case AST_BINARY_EXPR:
            emit_int_binary_expr_to_rax(ctx, node, mode);
            break;
        case AST_FUNCTION_CALL_EXPR:
            emit_function_call_expr(ctx, node, mode);
            break;
        case AST_ARRAY_ACCESS:
            emit_line(ctx, "; emitting array access");
            emit_addr_to_rax(ctx, node);

            if (node->ctype->kind != CTYPE_ARRAY) {
                emit_pop(ctx, "rcx");
                emit_line(ctx, "; emitting array element value");
                emit_load_from(ctx, node->ctype, "rcx");                    // load
                if (mode == WANT_VALUE) {
                    emit_push(ctx, "rax");
                }
            }

            break;
        case AST_CAST_EXPR:
            emit_int_cast_expr_to_rax(ctx, node, mode);
            break;
        case AST_STRING_LITERAL: {
            emit_addr_to_rax(ctx, node);
            // char * label = node->string_literal.label;
            // emit_line(ctx, "lea rax, [%s]", label);
            // emit_push(ctx, "rax");

            break;
        }
        default:
            error("Unexpected node type %d", get_ast_node_name(node));
    }

}
