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

void print_ast(ASTNode * node, int indent) {
    if(!node) return;

    print_indent(indent);

    switch(node->type) {
        case AST_TRANSLATION_UNIT:
        {
            printf("TranslationUnit:\n");

            ASTNode_list * globals = node->translation_unit.globals;
            for (ASTNode_list_node * n = globals->head ; n ; n = n->next) {
                print_ast(n->value, indent+1);
            }

            ASTNode_list * functions = node->translation_unit.functions;
            for (ASTNode_list_node * n = functions->head ; n ; n = n->next) {
                print_ast(n->value, indent+1);                
            }
            break;
        }
        case AST_FUNCTION_DECL:
            printf("FunctionDecl: %s, type: %s\n", node->function_decl.name, ctype_to_string(node->ctype));
            if (node->function_decl.param_list) {
                print_indent(indent+1); printf("ParameterList:\n");
                for (ASTNode_list_node * n = node->function_decl.param_list->head;n;n=n->next) {
                    print_ast(n->value, indent+2);
                }
            }
            if (node->function_decl.body) {
                print_ast(node->function_decl.body, indent+1);
            }
            break;
        case AST_FUNCTION_CALL:
            printf("FunctionCall: %s, type: %s\n", node->function_call.name, ctype_to_string(node->ctype));
            if (node->function_call.arg_list) {
                print_indent(indent+1); printf("ArgumentList:\n");
                for (ASTNode_list_node *n = node->function_call.arg_list->head;n;n=n->next) {
                    print_ast(n->value, indent+2);
                }
            }
            break;
        case AST_RETURN_STMT:
            printf("ReturnStmt: - type: %s\n", ctype_to_string(node->ctype));
            print_ast(node->return_stmt.expr, indent+1);
            break;
        case AST_INT_LITERAL:
            printf("IntLiteral: %d - type: %s\n", node->int_value, ctype_to_string(node->ctype));
            break;
        case AST_BINARY_EXPR:
            printf("BinaryExpr: %s - type: %s\n", get_binary_op_name(node->binary.op), ctype_to_string(node->ctype));
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;

        case AST_UNARY_EXPR:
            printf("UnaryExpr: %s - type: %s\n", get_unary_op_name(node->unary.op), ctype_to_string(node->ctype));
            print_ast(node->unary.operand, indent+1);
            break;
        case AST_CAST_EXPR:
            printf("CastExpr: target_type: %s, type: %s\n", ctype_to_string(node->cast_expr.target_type), ctype_to_string(node->ctype));
            print_ast(node->cast_expr.expr, indent+1);
            break;

        case AST_IF_STMT:
            printf("IfStmt: \n");
            print_ast(node->if_stmt.cond, indent+1);
            print_ast(node->if_stmt.then_stmt, indent+1);
            print_ast(node->if_stmt.else_stmt, indent+1);
            break;
        case AST_WHILE_STMT:
            printf("WhileStmt:\n");
            print_ast(node->while_stmt.cond, indent+1);
            print_ast(node->while_stmt.body, indent+1);
            break;
        case AST_FOR_STMT:
            printf("ForStmt:\n");
            if (node->for_stmt.init_expr) {
                print_indent(indent+1); printf("Init:\n");
                print_ast(node->for_stmt.init_expr, indent+2);
            }
            if (node->for_stmt.cond_expr) {
                print_indent(indent+1); printf("Condition:\n");
                print_ast(node->for_stmt.cond_expr, indent+2);
            }
            if (node->for_stmt.update_expr) {
                print_indent(indent+1); printf("Update:\n");
                print_ast(node->for_stmt.update_expr, indent+2);
            }
            print_indent(indent+1); printf("Body:\n");
            print_ast(node->for_stmt.body, indent+2);
            break;
        case AST_DO_WHILE_STMT:
            printf("DoWhileStmt:\n");
            print_ast(node->do_while_stmt.body, indent+1);
            print_ast(node->do_while_stmt.expr, indent+1);
            break;
        case AST_SWITCH_STMT:
            printf("SwitchStmt\n");
            print_ast(node->switch_stmt.expr, indent+1);
            print_ast(node->switch_stmt.stmt, indent+1);
            break;
        case AST_BLOCK:
            printf("BlockStmt\n");
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
                print_indent(indent+1); printf("VarInitializer:\n");
                print_ast(node->var_decl.init_expr, indent+2);
            }
            break;
        case AST_VAR_REF:
            printf("VariableExpression: %s, ctype: %s\n", node->var_ref.name, ctype_to_string(node->ctype));
            break;
        case AST_ARRAY_ACCESS:
            printf("ArrayAccess:\n");
            print_indent(indent+1); printf("ArrayExpression:\n");
            print_ast(node->array_access.base, indent+2);
            print_indent(indent+1); printf("IndexExpression:\n");
            print_ast(node->array_access.index, indent+2);
            break;
        case AST_INITIALIZER_LIST:
           printf("InitializerList:\n");
            // TODO
            break;
        default:        
            error("Unknown AST Node Type: %d\n", node->type);
            break;
    }
}
