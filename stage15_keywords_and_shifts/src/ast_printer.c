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

#include "parser_util.h"

void print_ast(ASTNode * node, int indent) {
    if(!node) return;

    print_indent(indent);

    char buf[128];
    char buf2[128];
    buf[0] = '\0';
    buf2[0] = '\0';

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
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));

            printf("FunctionDecl: %s, type: %s\n", node->function_decl.name, buf);
            if (node->function_decl.param_list) {
                print_indent(indent+1); printf("ParameterList:\n");
                for (ASTNode_list_node * n = node->function_decl.param_list->head;n;n=n->next) {
                    print_ast(n->value, indent+2);
                }
            }
            // if (node->function_decl.body) {
            //     print_ast(node->function_decl.body, indent+1);
            // }
            break;

    case AST_FUNCTION_DEF:
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));

            printf("FunctionDef: %s, type: %s\n", node->function_def.name, buf);
            if (node->function_def.param_list) {
                print_indent(indent+1); printf("ParameterList:\n");
                for (ASTNode_list_node * n = node->function_def.param_list->head;n;n=n->next) {
                    print_ast(n->value, indent+2);
                }
            }
            print_ast(node->function_def.body, indent+1);
            break;

        case AST_FUNCTION_CALL_EXPR:
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("FunctionCall: %s, type: %s\n", node->function_call.name, buf);
            if (node->function_call.arg_list) {
                print_indent(indent+1); printf("ArgumentList:\n");
                for (ASTNode_list_node *n = node->function_call.arg_list->head;n;n=n->next) {
                    print_ast(n->value, indent+2);
                }
            }
            break;
        case AST_RETURN_STMT: {
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("ReturnStmt: - type: %s\n", buf);
            print_ast(node->return_stmt.expr, indent+1);
            break;
        }
        case AST_BINARY_EXPR:
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("BinaryExpr: %s - type: %s\n", get_binary_op_name(node->binary.op), buf);
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;

        case AST_UNARY_EXPR:
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("UnaryExpr: %s - type: %s\n", get_unary_op_name(node->unary.op), buf);
            print_ast(node->unary.operand, indent+1);
            break;
        case AST_CAST_EXPR:
            ctype_to_cdecl(node->cast_expr.target_ctype, buf, sizeof(buf));
            ctype_to_cdecl(node->ctype, buf2, sizeof(buf2));
            printf("CastExpr: target_ctype: %s, type: %s\n", buf, buf2);
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
        case AST_BLOCK_STMT:
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
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("VariableDeclaration: %s, ctype: %s\n", node->var_decl.name, buf);
            if (node->var_decl.init_expr) {
                print_indent(indent+1); printf("VarInitializer:\n");
                print_ast(node->var_decl.init_expr, indent+2);
            }
            break;
        case AST_VAR_REF_EXPR:
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("VariableExpression: %s, ctype: %s\n", node->var_ref.name, buf);
            break;
        case AST_ARRAY_ACCESS:
            printf("ArrayAccess:\n");
            print_indent(indent+1); printf("ArrayExpression:\n");
            print_ast(node->array_access.base, indent+2);
            print_indent(indent+1); printf("IndexExpression:\n");
            print_ast(node->array_access.index, indent+2);
            break;
        case AST_INITIALIZER_LIST: {
            printf("InitializerList: [");
            for (ASTNode_list_node * n = node->initializer_list.items->head; n; n = n->next) {
                ASTNode * node = n->value;
                if (node->type == AST_INITIALIZER_LIST) {
//                    printf("\n");
                    print_ast(node, indent+1);
                }
                else {
                    if (n->value->type == AST_INT_LITERAL) {
                        printf("%d", n->value->int_value);
                    } else if (n->value->type == AST_FLOAT_LITERAL) {
                        printf("%f", n->value->float_literal.value);
                    } else if (n->value->type == AST_DOUBLE_LITERAL) {
                        printf("%f", n->value->double_literal.value);
                    } else if (n->value->type == AST_STRING_LITERAL) {
                        printf("%s", n->value->string_literal.value);
                    } else if (n->value->type == AST_UNARY_EXPR) {
                        printf("(unary expr)");
                    } else if (n->value->type == AST_CAST_EXPR) {
                        printf("Cast");
                    }
                    else {
                        error("Invalid initializer list type");
                    }
                    if (n->next != NULL) {
                        printf(", ");
                    }
                }
            }
            printf("]\n");
            break;
        }

        case AST_INT_LITERAL: {
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("IntLiteral: %d - type: %s\n", node->int_value, buf);
            break;
        }
        case AST_STRING_LITERAL:
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("StringLiteral: - type: %s, value: %s\n", buf, node->string_literal.value);
            break;
        case AST_FLOAT_LITERAL:
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("FloatLiteral: - type: %s, value: %f\n", buf, node->float_literal.value);
            break;
        case AST_DOUBLE_LITERAL:
            ctype_to_cdecl(node->ctype, buf, sizeof(buf));
            printf("DoubleLiteral: - type: %s, value: %lf\n", buf, node->double_literal.value);
            break;
        case AST_DECLARATION_STMT:
            printf("DeclarationStmt:\n");
            for (ASTNode_list_node * n = node->declaration.init_declarator_list->head; n; n = n->next) {
                print_ast(n->value, indent+1);
            }
            break;
        default:
            error("Unknown AST Node Type: %d\n", get_ast_node_name(node));
            break;
    }
}
