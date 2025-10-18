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
#include "emit_condition.h"
#include "emit_stack.h"

#include "error.h"

INTERNAL void emit_function_call_expr(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    // if the call has arguments
    // first get a reversed list
    // then emit each arg then push it
    //struct node_list * reversed_list = NULL;
    int stack_args_size = 0;
    int arg_count=0;
    if (node->function_call.arg_list) {

        // loop through in reverse order pushing arguments to the stack
        for (int i = node->function_call.arg_list->count - 1; i >= 0; i--) {
            ASTNode * argNode = ASTNode_list_get(node->function_call.arg_list, i);
            if (is_floating_point_type(argNode->ctype)) {
                emit_fp_expr_to_xmm0(ctx, argNode, WANT_VALUE);
                stack_args_size += 16;
            }
            else {
                emit_int_expr_to_rax(ctx, argNode, WANT_VALUE);
                stack_args_size += 8;
            }
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
//        emit_add_rsp(ctx, arg_count*8);
        emit_add_rsp(ctx, stack_args_size);
    }

    //    emit_line(ctx, "push rax");
    if (node->ctype->kind != CTYPE_VOID && mode == WANT_VALUE) {
        if (is_floating_point_type(node->ctype)) {
            emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
        }
        else {
            emit_push(ctx, "rax");
        }
    }

}

INTERNAL void emit_cast_expr(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_line(ctx, "; emitting cast");
    //emit_tree_node(ctx, node->cast_expr.expr);
    emit_expr_to_reg(ctx, node->cast_expr.expr, mode);
    if (node->ctype->kind == CTYPE_DOUBLE) {
        switch (node->cast_expr.expr->ctype->kind) {
            case CTYPE_FLOAT:
                if (mode == WANT_VALUE) {
                    emit_fpop(ctx, "xmm0", FP32);
                }
                emit_line(ctx, "cvtss2sd %s, %s", "xmm0", "xmm0    ; float -> double");
                break;
            case CTYPE_INT:
                if (mode == WANT_VALUE) {
                    emit_pop(ctx, "RAX");
                }
                emit_line(ctx, "cvtsi2sd %s, %s", "xmm0", "eax    ; int -> double");

                break;
            case CTYPE_LONG:
                if (mode == WANT_VALUE) {
                    emit_pop(ctx, "RAX");
                }

                emit_line(ctx, "cvtsi2sd %s, %s", "xmm0", "rax    ; long -> double");
                break;
            default: error("Unsupported cast expression type %d", node->type);
        }
        if (mode == WANT_VALUE) {
            emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
        }
    }
    else if (node->ctype->kind == CTYPE_FLOAT) {
        switch (node->cast_expr.expr->ctype->kind) {
            case CTYPE_DOUBLE:
                if (mode == WANT_VALUE) {
                    emit_fpop(ctx, "xmm0", FP64);
                }
                emit_line(ctx, "cvtsd2ss %s, %s", "xmm0", "xmm0     ; double -> float"    );
                break;
            case CTYPE_INT:
                if (mode == WANT_VALUE) {
                    emit_pop(ctx, "rax");
                }
                emit_line(ctx, "cvtsi2ss %s, %s", "xmm0", "eax     ; int -> float");
                break;
            case CTYPE_LONG:
                if (mode == WANT_VALUE) {
                    emit_pop(ctx, "rax");
                }
                emit_line(ctx, "cvtsi2ss %s, %s", "xmm0", "rax    ; long -> float");
                break;
            default: error("Unsupported cast expression type %d", node->type);
        }
        if (mode == WANT_VALUE) {
            emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
        }

    }
}

INTERNAL void emit_int_cast_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {

    if (is_floating_point_type(node->cast_expr.expr->ctype)) {
        emit_fp_expr_to_xmm0(ctx, node->cast_expr.expr, mode);

        if (mode == WANT_VALUE) {
            emit_fpop(ctx, stack_reg_for_type(node->cast_expr.expr->ctype), getFPWidthFromCType(node->cast_expr.expr->ctype));
        }

        if (node->cast_expr.expr->ctype->kind == CTYPE_FLOAT) {
            emit_line(ctx, "cvttss2si %s, %s", reg_for_type(node->ctype), reg_for_type(node->cast_expr.expr->ctype));
        } else if (node->cast_expr.expr->ctype->kind == CTYPE_DOUBLE) {
            emit_line(ctx, "cvttsd2si %s, %s", reg_for_type(node->ctype), reg_for_type(node->cast_expr.expr->ctype));
        }
        else {
            error("Invalid Type");
        }

        if (mode == WANT_VALUE) {
            emit_push(ctx, stack_reg_for_type(node->ctype));
        }
        return;
    }

    emit_int_expr_to_rax(ctx, node->cast_expr.expr, WANT_VALUE);     // eval inner expression

    CType * from_type = node->cast_expr.expr->ctype;
    CType * to_type = node->cast_expr.target_ctype;

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

void emit_int_assignment_expr_to_rax(EmitterContext * ctx, ASTNode* node, EvalMode mode) {
    emit_line(ctx, "; emitting assignment - LHS %s = RHS %s",
        get_ast_node_name(node->binary.lhs), get_ast_node_name(node->binary.rhs));
    // eval RHS -> rax then push
    if (is_array_type(node->binary.rhs->ctype)) {
        emit_addr_to_rax(ctx, node->binary.rhs, WANT_VALUE);
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

INTERNAL void emit_int_ptr_add_expr_to_rax(EmitterContext * ctx, ASTNode* node, EvalMode mode) {
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
    else {
        error("Unsupported types for binary pointer add operation");
    }
    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
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

    // if (lhs_type->kind == CTYPE_PTR && is_integer_type(rhs_type)) {
    //     int elem_size = sizeof_type(lhs_type->base_type);
    //     emit_line(ctx, "imul rcx, %d", elem_size);
    //     emit_line(ctx, "add rax, rcx");
    // }
    // else if (is_integer_type(lhs_type) && rhs_type->kind == CTYPE_PTR) {
    //     int elem_size = sizeof_type(rhs_type->base_type);
    //     emit_line(ctx, "imul rax, %d", elem_size);
    //     emit_line(ctx, "add rax, rcx");
    // }
    //else
    if (is_integer_type(lhs_type) && is_integer_type(rhs_type)) {
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
    int label_true = get_label_id(ctx);
    int label_false = get_label_id(ctx);
    int label_end = get_label_id(ctx);

    emit_condition(ctx, node, label_true, label_false, mode);

    emit_label(ctx, "Lcond", label_true);
    emit_line(ctx, "mov eax, 1");
    emit_jump(ctx, "jmp", "Lcond", label_end);

    emit_label(ctx, "Lcond", label_false);
    emit_line(ctx, "xor eax, eax");

    emit_label(ctx, "Lcond", label_end);

    // //lhs
    // emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);
    // emit_pop(ctx, "rax");
    // emit_line(ctx, "cmp eax, 0");
    // emit_jump(ctx, "je", "false", label_false);
    //
    // //rhs
    // emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);
    // emit_pop(ctx, "rax");
    // emit_line(ctx, "cmp eax, 0");
    // emit_jump(ctx, "je", "false", label_false);
    //
    // // both true
    // emit_line(ctx, "mov eax, 1");
    // emit_jump(ctx, "jmp", "end", label_end);
    //
    // emit_label(ctx, "false", label_false);
    // emit_line(ctx, "mov eax, 0");

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }

//    emit_label(ctx, "end", label_end);
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
    emit_pop(ctx, "rcx");

    // restore LHS into eax
    emit_pop(ctx, "rax");


    // add RHS to LHS
    emit_line(ctx, "add eax, ecx");

    // restore LHS address
    emit_pop(ctx, "rcx");


    // write back result to LHS
    emit_line(ctx, "mov [rcx], eax");

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
}

void emit_fp_assignment_expr_to_xmm0(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_line(ctx, "; emitting assignment - LHS %s = RHS %s",
    get_ast_node_name(node->binary.lhs), get_ast_node_name(node->binary.rhs));
    // eval RHS -> rax then push
    if (is_array_type(node->binary.rhs->ctype)) {
        emit_addr_to_rax(ctx, node->binary.rhs, WANT_VALUE);
    }
    else {
        emit_fp_expr_to_xmm0(ctx, node->binary.rhs, WANT_VALUE);
    }

    // eval LHS addr -> rcx
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_ADDRESS);

    // pop LHS into rcx
    emit_pop(ctx, "rcx");

    // pop RHS in rax
    if (is_array_type(node->binary.rhs->ctype)) {
        emit_pop(ctx, "rax");
    }
    else {
        emit_fpop(ctx, "xmm0", getFPWidthFromCType(node->binary.rhs->ctype));
    }

    CType * ctype = node->binary.lhs->ctype;

    emit_line(ctx, "%s %s [rcx], %s",
        mov_instruction_for_type(ctype),
        mem_size_for_type(ctype),
        reg_for_type(ctype));


}

void emit_fp_add_assignment_expr_to_xmm0(EmitterContext * ctx, ASTNode * node, EvalMode mode) {

    // compute lhs address -> rcx
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_ADDRESS);

    emit_pop(ctx, "rcx");               // pop rcx off the stack to load the lhs value
    emit_push(ctx, "rcx");              // push rcx back on the stack to latter store the result
    // load lhs into rax
    emit_line(ctx, "%s xmm0, [rcx]", mov_instruction_for_type(node->binary.lhs->ctype));
    //    emit_line(ctx, "push rax");
    emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->binary.lhs->ctype));


    // evaluate RHS into eax
    emit_expr_to_reg(ctx, node->binary.rhs, WANT_VALUE);

    // restore RHS into ecx
    //    emit_line(ctx, "pop rcx");
    emit_fpop(ctx, "xmm1", getFPWidthFromCType(node->binary.rhs->ctype));

    // restore LHS into eax
    //    emit_line(ctx, "pop rax");
    emit_fpop(ctx, "xmm0", getFPWidthFromCType(node->binary.lhs->ctype));

    // add RHS to LHS
    emit_line(ctx, "%s xmm0, xmm1", get_fp_binop(node->binary.lhs));

    // restore LHS address
    //    emit_line(ctx, "pop rcx");
    emit_pop(ctx, "rcx");


    // write back result to LHS
    emit_line(ctx, "%s [rcx], xmm0", mov_instruction_for_type(node->binary.lhs->ctype));

    if (mode == WANT_VALUE) {
        emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->binary.lhs->ctype));
    }
}


INTERNAL void emit_int_sub_assignment_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {

    // compute lhs address -> rcx
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_ADDRESS);

    emit_pop(ctx, "rcx");               // pop rcx off the stack to load the lhs value
    emit_push(ctx, "rcx");              // push rcx back on the stack to latter store the result

    // push LHS address on stack
    emit_line(ctx, "mov DWORD eax, [rcx]");
    emit_push(ctx, "rax");


    // // load current value into eax
    // emit_line(ctx, "mov eax, [rcx]");
    //
    // // save current RHS value in eax on stack
    // emit_line(ctx, "push rax");

    // evaluate RHS into eax
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);


    // restore RHS into ecx
    emit_pop(ctx, "rcx");

    // restore LHS into eax
    emit_pop(ctx, "rax");

    // sub RHS from LHS
    emit_line(ctx, "sub eax, ecx");

    // restore LHS address
    emit_pop(ctx, "rcx");

    // write back result to LHS
    emit_line(ctx, "mov [rcx], eax");

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
}

void emit_signed_integer_condition_codes(EmitterContext * ctx, BinaryOperator op) {
    // emit proper setX based on operator type
    switch (op) {
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
}

void emit_unsigned_integer_condition_codes(EmitterContext * ctx, BinaryOperator op) {
    // emit proper setX based on operator type
    switch (op) {
        case BINOP_EQ:
            emit_line(ctx, "sete al");
            break;

        case BINOP_NE:
            emit_line(ctx, "setne al");
            break;

        case BINOP_LT:
            emit_line(ctx, "setb al");
            break;

        case BINOP_LE:
            emit_line(ctx, "setbe al");
            break;

        case BINOP_GT:
            emit_line(ctx, "seta al");
            break;

        case BINOP_GE:
            emit_line(ctx, "setae al");
            break;

        default:
            error("Unsupported comparison type in codegen.");
    }

    // zero-extend result to full eax
    emit_line(ctx, "movzx eax, al");
}

void emit_floating_point_condition_codes(EmitterContext * ctx, BinaryOperator op) {
    // emit proper setX based on operator type
    switch (op) {
        case BINOP_EQ:
            emit_line(ctx, "sete al           ; al = ZF ");
            emit_line(ctx, "setnp dl          ; dl = !PF ");
            emit_line(ctx, "and al, dl        ; exclude NaN");
            break;

        case BINOP_NE:
            emit_line(ctx, "setne al          ; al = !ZF");
            emit_line(ctx, "setp dl           ; dl = PF (unordered)");
            emit_line(ctx, "or al, dl");
            break;

        case BINOP_LT:
            emit_line(ctx, "setb al           ; al = CF");
            emit_line(ctx, "setnp dl          ; dl = !PF (ordered)");
            emit_line(ctx, "and al, dl        ; al = CF && !PF");
            break;

        case BINOP_LE:
            emit_line(ctx, "setbe al          ; al = CF|ZF");
            emit_line(ctx, "setnp dl          ; dl = !PF");
            emit_line(ctx, "and al, dl");
            break;

        case BINOP_GT:
            emit_line(ctx, "seta al           ; al = !(CF|ZF)");
            emit_line(ctx, "setnp dl");
            emit_line(ctx, "and al, dl");
            break;

        case BINOP_GE:
            emit_line(ctx, "setae al         ; al = !CF");
            emit_line(ctx, "setnp dl");
            emit_line(ctx, "and al, dl");
            break;

        default:
            error("Unsupported comparison type in codegen.");
    }

    // zero-extend result to full eax
    emit_line(ctx, "movzx eax, al");
}

INTERNAL void emit_int_comparison_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    // eval left-hand side -> result in eax -> push results onto the stack
    if (is_floating_point_type(node->binary.common_type)) {
        emit_fp_expr_to_xmm0(ctx, node->binary.lhs, WANT_VALUE);
        emit_fp_expr_to_xmm0(ctx, node->binary.rhs, WANT_VALUE);
    }
    else {
        emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);
        emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);
    }

    // emit_line(ctx, "pop rcx");
    // emit_line(ctx, "pop rax");
    if (is_floating_point_type(node->binary.common_type)) {
        emit_fpop(ctx, "xmm1", getFPWidthFromCType(node->binary.common_type));
        emit_fpop(ctx, "xmm0", getFPWidthFromCType(node->binary.common_type));
    }
    else {
        emit_pop(ctx, "rcx");
        emit_pop(ctx, "rax");
    }

    // eval right-hand side -> reult in eax

    // restore lhs into rcx

    if (!is_floating_point_type(node->binary.common_type)) {
        emit_line(ctx, "mov ecx, ecx");   // zero upper bits
        emit_line(ctx, "mov eax, eax");   // zero upper bits
    }

    // compare rax (lhs) with ecx (rhs), cmp rcx, eax means rcx - eax
    if (node->binary.common_type->kind == CTYPE_FLOAT) {
        emit_line(ctx, "ucomiss xmm0, xmm1     ; fp compare");
    } else if (node->binary.common_type->kind == CTYPE_DOUBLE) {
        emit_line(ctx, "ucomisd xmm0, xmm1     ; dbl compare");
    } else {
        emit_line(ctx, "cmp eax, ecx");
    }

    if (is_signed_integer_type(node->binary.common_type) || is_pointer_type(node->binary.common_type)) {
        emit_signed_integer_condition_codes(ctx, node->binary.op);
    } else if (is_unsigned_integer_type(node->binary.common_type)) {
        emit_unsigned_integer_condition_codes(ctx, node->binary.op);

    } else if (is_floating_point_type(node->binary.common_type)) {
        emit_floating_point_condition_codes(ctx, node->binary.op);
    }

    // // emit proper setX based on operator type
    // switch (node->binary.op) {
    //     case BINOP_EQ:
    //         emit_line(ctx, "sete al");
    //         break;
    //
    //     case BINOP_NE:
    //         emit_line(ctx, "setne al");
    //         break;
    //
    //     case BINOP_LT:
    //         emit_line(ctx, "setl al");
    //         break;
    //
    //     case BINOP_LE:
    //         emit_line(ctx, "setle al");
    //         break;
    //
    //     case BINOP_GT:
    //         emit_line(ctx, "setg al");
    //         break;
    //
    //     case BINOP_GE:
    //         emit_line(ctx, "setge al");
    //         break;
    //
    //     default:
    //         error("Unsupported comparison type in codegen.");
    // }
    //
    // // zero-extend result to full eax
    // emit_line(ctx, "movzx eax, al");
    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
}


void emit_int_binary_expr_to_rax(EmitterContext * ctx, ASTNode *node, EvalMode mode) {
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
            if (is_pointer_type(node->binary.lhs->ctype) || is_pointer_type(node->binary.rhs->ctype)) {
                emit_int_ptr_add_expr_to_rax(ctx, node, mode);
            }
            else if (is_integer_type(node->ctype)) {
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
        case BINOP_BITWISE_AND:
        case BINOP_BITWISE_OR:
        case BINOP_BITWISE_XOR:
            emit_bitwise_binary_expr(ctx, node, mode);
            break;
        case BINOP_SHIFT_LEFT:
        case BINOP_SHIFT_RIGHT:
            emit_shift_expr(ctx, node, mode);
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
        case UNARY_LOGICAL_NOT:
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
        case UNARY_BITWISE_NOT:
            emit_int_expr_to_rax(ctx, node->unary.operand, WANT_VALUE);
            emit_pop(ctx, "rax");
            emit_line(ctx, "not eax");
            if (mode == WANT_VALUE) {
                emit_push(ctx, "rax");
            }

            break;
        default:
            error("Unsupported unary op in emitter");
    }
}



void emit_fp_arith_expr_to_xmm0(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_fp_expr_to_xmm0(ctx, node->binary.lhs, WANT_VALUE);       // codegen to eval lhs with result in EAX
    emit_fp_expr_to_xmm0(ctx, node->binary.rhs, WANT_VALUE);

    FPWidth width = getFPWidthFromCType(node->ctype);
    const char * instruction = get_fp_binop(node);

    emit_fpop(ctx, "xmm1", width);
    emit_fpop(ctx, "xmm0", width);

    emit_line(ctx, "%s xmm0, xmm1", instruction);

    if (mode == WANT_VALUE) {
        emit_fpush(ctx, "xmm0", width);
    }
}

void emit_fp_unary_expr_to_xmm0(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    switch (node->unary.op) {
        case UNARY_NEGATE:
            emit_fp_expr_to_xmm0(ctx, node->unary.operand, WANT_VALUE);
            //            emit_line(ctx, "pop rax");
            emit_fpop(ctx, "xmm0", getFPWidthFromCType(node->ctype));

            if (node->ctype->kind == CTYPE_FLOAT) {
//                emit_line(ctx, "movaps    xmm1, [rel mask_f32]          ; mask = 0x80000000");
                emit_line(ctx, "movups    xmm1, [rel mask_f32]          ; mask = 0x80000000");
                emit_line(ctx, "xorps     xmm0, xmm1                ; flips sign bit");
            }
            else if (node->ctype->kind == CTYPE_DOUBLE) {
//                emit_line(ctx, "movapd    xmm1, [rel mask_f64]          ; mask = 0x8000000000000000");
                emit_line(ctx, "movupd    xmm1, [rel mask_f64]          ; mask = 0x8000000000000000");
                emit_line(ctx, "xorpd     xmm0, xmm1                ; flips sign bit");
            }
            if (mode == WANT_VALUE) {
                emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
            }
            break;
        default:
            error("Unsupported unary op in emitter");
    }

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
        case BINOP_ADD:
        case BINOP_MUL:
        case BINOP_DIV:
        case BINOP_SUB: {
            if (is_floating_point_type(node->ctype)) {
                emit_fp_arith_expr_to_xmm0(ctx, node, mode);
            }
            break;
        }

            //TODO Need something to handle the assignments include compound like
            // a floating point version of emit_int_sub_assignment_expr_to_rax
            // or a more general version that handles both

        case BINOP_ASSIGNMENT:
            emit_fp_assignment_expr_to_xmm0(ctx, node, mode);
            break;
        case BINOP_COMPOUND_ADD_ASSIGN:
            emit_fp_add_assignment_expr_to_xmm0(ctx, node, mode);
//            emit_int_add_assignment_expr_to_rax(ctx, node, mode);
            break;
        case BINOP_COMPOUND_SUB_ASSIGN:
            emit_int_sub_assignment_expr_to_rax(ctx, node, mode);
            break;

        // case BINOP_MUL: {
        //     emit_fp_multi_expr_to_xmm0(ctx, node, mode);
        //     break;
        // }
        // case BINOP_SUB: {
        //     emit_fp_subtraction_expr_to_xmm0(ctx, node, mode);
        //     break;
        // }
        // case BINOP_DIV: {
        //     emit_fp_divide_expr_to_xmm0(ctx, node, mode);
        //     break;
        // }
        default:
            error("Unexpected node type %d", get_ast_node_name(node));
    }

}

void emit_binary_expr_to_reg(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    if (is_floating_point_type(node->binary.lhs->ctype)) {
        emit_fp_binary_expr_to_xmm0(ctx, node, mode);
    }
    else {
        emit_int_binary_expr_to_rax(ctx, node, mode);
    }
}

void emit_fp_expr_to_xmm0(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    if (mode == WANT_ADDRESS) {
        emit_addr_to_rax(ctx, node, mode);
        return;
    }
    switch (node->type) {

        case AST_CAST_EXPR: {
             emit_cast_expr(ctx, node, mode);
            break;
        }

        case AST_FLOAT_LITERAL:
            emit_line(ctx, "movss xmm0, [rel %s]", node->float_literal.label);  // need label
            if (mode == WANT_VALUE) {
                emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
            }
            break;

        case AST_DOUBLE_LITERAL:
            emit_line(ctx, "movsd xmm0, [rel %s]", node->double_literal.label);  // need label
            if (mode == WANT_VALUE) {
                emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
            }
            break;

        case AST_BINARY_EXPR:
            emit_fp_binary_expr_to_xmm0(ctx, node, mode);
            break;

        case AST_UNARY_EXPR:
            emit_fp_unary_expr_to_xmm0(ctx, node, mode);
            break;

        case AST_VAR_REF_EXPR:
            emit_addr_to_rax(ctx, node, mode);
            if (!is_array_type(node->ctype)) {
                if (is_floating_point_type(node->ctype)) {
                    //emit_fpop(ctx, "xmm0", FP32);
                    if (mode == WANT_VALUE) {
                        emit_pop(ctx, "rax");
                    }
                    emit_load_from(ctx, node->ctype, "rax");
                    if (mode == WANT_VALUE) {
                        emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
                    }
                } else {
                    if (mode == WANT_VALUE) {
                        emit_pop(ctx, "rax");
                    }
                    emit_load_from(ctx, node->ctype, "rax");
                    if (mode == WANT_VALUE) {
                        emit_push(ctx, "rax");
                    }
                }
            }
            break;

        case AST_FUNCTION_CALL_EXPR:
            emit_function_call_expr(ctx, node, mode);
            break;

        case AST_ARRAY_ACCESS:
            emit_line(ctx, "; emitting array access");
            emit_addr_to_rax(ctx, node, mode);

            if (node->ctype->kind != CTYPE_ARRAY) {
                emit_pop(ctx, "rcx");
                emit_line(ctx, "; emitting array element value");
                emit_load_from(ctx, node->ctype, "rcx");                    // load
                if (mode == WANT_VALUE) {
                    emit_push_for_type(ctx, node->ctype);
                }
            }

            break;

        case AST_COND_EXPR: {
            int id = get_label_id(ctx);
            emit_line(ctx, "; emitting conditional expression");
            emit_int_expr_to_rax(ctx, node->cond_expr.cond, mode);
            emit_pop(ctx, "rax");
            emit_line(ctx, "cmp eax, 0");

            emit_line(ctx, "je Lelse%d", id);
            emit_line(ctx, "; emitting then expr");
            emit_fp_expr_to_xmm0(ctx, node->cond_expr.then_expr, mode);
            emit_fpop(ctx, "xmm0", getFPWidthFromCType(node->ctype));
            emit_line(ctx, "jmp Lend%d", id);
            emit_label(ctx, "else", id);
            emit_line(ctx, "; emitting else expr");
            emit_fp_expr_to_xmm0(ctx, node->cond_expr.else_expr, mode);
            emit_fpop(ctx, "xmm0", getFPWidthFromCType(node->ctype));

            emit_label(ctx, "end", id);

            if (mode == WANT_VALUE) {
                emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype) );
            }

            break;
        }
        default:
            error("Unexpected node type %d", get_ast_node_name(node));
    }
}

// emit_expr.
// generate code to eval the expression storing the final result in eax or rax
void emit_int_expr_to_rax(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    if (mode == WANT_ADDRESS) {
        emit_addr_to_rax(ctx, node, mode);
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
                emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
            }
            break;

        case AST_DOUBLE_LITERAL:
            emit_line(ctx, "movsd xmm0, [rel %s]", node->double_literal.label);  // need label
            if (mode == WANT_VALUE) {
                emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
            }
            break;

        case AST_VAR_REF_EXPR: {
            emit_addr_to_rax(ctx, node, mode);
            if (!is_array_type(node->ctype)) {
                if (is_floating_point_type(node->ctype)) {
                    //emit_fpop(ctx, "xmm0", FP32);
                    if (mode == WANT_VALUE) {
                        emit_pop(ctx, "rax");
                    }
                    emit_load_from(ctx, node->ctype, "rax");
                    if (mode == WANT_VALUE) {
                        emit_fpush(ctx, "xmm0", getFPWidthFromCType(node->ctype));
                    }
                } else {
                    if (mode == WANT_VALUE) {
                        emit_pop(ctx, "rax");
                    }
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
            emit_addr_to_rax(ctx, node, mode);

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

        case AST_COND_EXPR: {
            int id = get_label_id(ctx);
            emit_line(ctx, "; emitting conditional expression");
            emit_int_expr_to_rax(ctx, node->cond_expr.cond, mode);
            emit_pop(ctx, "rax");
            emit_line(ctx, "cmp eax, 0");

            emit_line(ctx, "je Lelse%d", id);
            emit_line(ctx, "; emitting then expr");
            emit_int_expr_to_rax(ctx, node->cond_expr.then_expr, mode);
            emit_pop(ctx, "rax");
            emit_line(ctx, "jmp Lend%d", id);
            emit_label(ctx, "else", id);
            emit_line(ctx, "; emitting else expr");
            emit_int_expr_to_rax(ctx, node->cond_expr.else_expr, mode);
            emit_pop(ctx, "rax");

            emit_label(ctx, "end", id);

            if (mode == WANT_VALUE) {
                emit_push(ctx, "rax");
            }

            break;
        }
        case AST_STRING_LITERAL: {
            emit_addr_to_rax(ctx, node, mode);
            // char * label = node->string_literal.label;
            // emit_line(ctx, "lea rax, [%s]", label);
            // emit_push(ctx, "rax");

            break;
        }
        default:
            error("Unexpected node type %d", get_ast_node_name(node));
    }

}

void emit_expr_to_reg(EmitterContext * ctx, ASTNode * node, EvalMode mode) {

    bool is_floating_point = false;
    if (node->ctype) {
        is_floating_point = is_floating_point_type(node->ctype);
    }
    else if (is_assignment(node) && is_floating_point_type(node->expr_stmt.expr->binary.lhs->ctype) /*||
                is_floating_point_type(node->expr_stmt.expr->binary.rhs->ctype) */ ) {
        is_floating_point = true;
    }

    if (is_floating_point) {
        emit_fp_expr_to_xmm0(ctx, node, mode);
    } else {
        emit_int_expr_to_rax(ctx, node, mode);
    }
}

void emit_bitwise_binary_expr(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);       // codegen to eval lhs with result in EAX
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);       // codegen to eval rhs with result in EAX

    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");

    switch (node->binary.op) {
        case BINOP_BITWISE_AND:
            emit_line(ctx, "and eax, ecx");
            break;
        case BINOP_BITWISE_OR:
            emit_line(ctx, "or eax, ecx");
            break;
        case BINOP_BITWISE_XOR:
            emit_line(ctx, "xor eax, ecx");
            break;
        default:
            error("invalid bitwise operator");
    }

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }
}

void emit_shift_expr(EmitterContext * ctx, ASTNode * node, EvalMode mode) {
    emit_int_expr_to_rax(ctx, node->binary.lhs, WANT_VALUE);       // codegen to eval lhs with result in EAX
    emit_int_expr_to_rax(ctx, node->binary.rhs, WANT_VALUE);       // codegen to eval rhs with result in EAX

    emit_pop(ctx, "rcx");
    emit_pop(ctx, "rax");

    switch (node->binary.op) {
        case BINOP_SHIFT_LEFT:
            emit_line(ctx, "shl eax, cl");
            break;
            case BINOP_SHIFT_RIGHT:
            emit_line(ctx, "sar eax, cl");
            break;
    }

    if (mode == WANT_VALUE) {
        emit_push(ctx, "rax");
    }

}