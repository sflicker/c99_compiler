#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"
#include "util.h"
#include "token.h"
#include "ast.h"
#include "ast_list.h"
#include "parser.h"
#include "ctype.h"
#include "ast_printer.h"

void print_indent(int indent) {
    for (int i=0;i<indent;i++) printf("  ");
}

void print_ast(ASTNode * node, int indent) {
    if(!node) return;

    print_indent(indent);

    switch(node->type) {
        case AST_TRANSLATION_UNIT:
        {
            printf("TranslationUnit:\n");
            ASTNode_list * functions = node->translation_unit.functions;
            for (ASTNode_list_node * n = functions->head ; n ; n = n->next) {
                print_ast(n->value, indent+1);                
            }
            // for (int i=0;i<node->translation_unit.count;i++) {
            //    print_ast(node->translation_unit.functions[i], indent+1);
            // }   
            break;
        }
        case AST_FUNCTION_DECL:
            printf("FunctionDecl: %s, size: %d\n", node->function_decl.name, node->function_decl.size);
            if (node->function_decl.param_list) {
                print_indent(indent+1); printf("ParameterList:\n");
                for (ASTNode_list_node * n = node->function_decl.param_list->head;n;n=n->next) {
                    print_ast(n->value, indent+2);
                }
            }
           // if (node->function_decl.param_list) {
                //TODO print the param_list
                //print_ast(node->function_decl.param_list, indent+1);
                // struct node_list * node_list = node->function_decl.param_list;
                // while(node_list) {
                //     print_ast(node_list->node, indent+1);
                //     node_list = node_list->next;
                // }
           // }
            if (node->function_decl.body) {
                print_ast(node->function_decl.body, indent+1);
            }
            break;
        //case AST_PARAM_LIST: {
          //   printf("ParameterList:\n");
            //  ASTNode * paramCurr = node;
            //  while(paramCurr) {
            //      print_ast(paramCurr->param_list.param, indent + 1);
            //      paramCurr = paramCurr->param_list.next;
            //  }
            //  break;
            // struct node_list * node_list = node->param_list.node_list;
            // while(node_list) {
            //     print_ast(node_list->node, indent+1);
            //     node_list = node_list->next;
            // }
        //    break;
        //}
        case AST_FUNCTION_CALL:
            printf("FunctionCall: %s\n", node->function_call.name);
            if (node->function_call.arg_list) {
                print_indent(indent+1); printf("ArgumentList:\n");
                for (ASTNode_list_node *n = node->function_call.arg_list->head;n;n=n->next) {
                    print_ast(n->value, indent+2);
                }
            }
            //if (node->function_call.argument_expression_list) {
                //TODO PRINT ARG LIST
                // struct node_list * arg = node->function_call.argument_expression_list;
                // while(arg) {
                //     print_ast(arg->node, indent + 1);
                //     arg = arg->next;
                // }
            //}
            break;
        // case AST_ARG_LIST: {
        //     printf("ArgumentList:\n");
        //     struct node_list * arg = node;
        //     while(arg) {
        //          print_ast(arg->argument_list.expression, indent + 1);
        //          arg = arg->argument_list.next;
        //      }
        //      break;
        // }
        case AST_RETURN_STMT:
            printf("ReturnStmt:\n");
            print_ast(node->return_stmt.expr, indent+1);
            break;
        case AST_INT_LITERAL:
            printf("IntLiteral: %d\n", node->int_value);
            break;
        case AST_BINARY_EXPR:
            printf("Binary: %s\n", get_binary_op_name(node->binary.op));
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;

        case AST_UNARY_EXPR:
            printf("Unary: %s\n", get_unary_op_name(node->unary.op));
            print_ast(node->unary.operand, indent+1);
            break;

        case AST_IF_STMT:
            printf("IfStmt: \n");
            print_ast(node->if_stmt.cond, indent+1);
            print_ast(node->if_stmt.then_statement, indent+1);
            print_ast(node->if_stmt.else_statement, indent+1);
            break;
        case AST_WHILE_STMT:
            printf("WhileStmt:\n");
            print_ast(node->while_stmt.cond, indent+1);
            print_ast(node->while_stmt.body, indent+1);
            break;
        case AST_FOR_STMT:
            printf("ForStmt:\n");
            if (node->for_stmt.init_expr) {
                print_ast(node->for_stmt.init_expr, indent+1);
            }
            if (node->for_stmt.cond_expr) {
                print_ast(node->for_stmt.cond_expr, indent+1);
            }
            if (node->for_stmt.update_expr) {
                print_ast(node->for_stmt.update_expr, indent+1);
            }
            print_ast(node->for_stmt.body, indent+1);
            break;
        case AST_DO_WHILE_STMT:
            printf("DoWhileStmt:\n");
            print_ast(node->do_while_stmt.stmt, indent+1);
            print_ast(node->do_while_stmt.expr, indent+1);
            break;
        case AST_SWITCH_STMT:
            printf("SwitchStatement\n");
            print_ast(node->switch_stmt.expr, indent+1);
            print_ast(node->switch_stmt.stmt, indent+1);
            break;
        case AST_BLOCK:
            printf("Block\n");
            for (ASTNode_list_node * n = node->block.statements->head; n; n = n->next) {
                print_ast(n->value, indent+1);
            }
            break;
        case AST_BREAK_STMT:
            printf("BreakStatment:\n");
            break;
        case AST_CONTINUE_STMT:
            printf("ContinueStmt:\n");
            break;
        case AST_GOTO_STMT:
            printf("GotoStatment: %s\n", node->goto_stmt.label);
            break;
        case AST_CASE_STMT:
            printf("CaseStatement %d\n", node->case_stmt.constExpression->int_value);
            print_ast(node->case_stmt.stmt, indent+1);
            break;
        case AST_LABELED_STMT:
            printf("LabeledStatement: %s\n", node->labeled_stmt.label);
            print_ast(node->labeled_stmt.stmt, indent+1);
            break;
        case AST_DEFAULT_STMT:
            printf("Default:\n");
            print_ast(node->default_stmt.stmt, indent+1);
            break;
        case AST_EXPRESSION_STMT:
            printf("ExpressionStatement\n");
            print_ast(node->expr_stmt.expr, indent+1);
            break;
        case AST_ASSERT_EXTENSION_STATEMENT:
            printf("AssertStatement\n");
            print_ast(node->expr_stmt.expr, indent+1);
            break;
        case AST_PRINT_EXTENSION_STATEMENT:
            printf("PrintExtensionStatement\n");
            print_ast(node->expr_stmt.expr, indent+1);
            break;
        case AST_VAR_DECL:
            printf("VariableDeclaration: %s, ctype: %s\n", node->var_decl.name, ctype_to_string(node->ctype));
            if (node->var_decl.init_expr) {
                print_ast(node->var_decl.init_expr, indent+1);
            }
            break;
        case AST_VAR_REF:
            printf("VariableExpression: %s\n", node->var_ref.name);
            break;
        default:        
            error("Unknown AST Node Type: %d\n", node->type);
            break;
    }
}
