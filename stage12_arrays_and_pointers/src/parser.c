#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdbool.h>

#include "list_util.h"
#include "token.h"
#include "ast.h"
#include "error.h"
#include "parser.h"
#include "parser_util.h"
#include "parser_context.h"
#include "c_type.h"

/* 
   Simple C compiler example 

   Simplified grammar

   <translation-unit>  ::= <external-declaration>
                         | <translation-unit> <external-declaration>

   <external-declaration>   ::= <function-definition>
                               | <global_variable_declaration>

   <function-definition> ::= <type-specifier> <identifier> "(" <parameter_list>* ")"
                [ <block> | ";" ]

   <global_variable_declaration> ::= <var_declaration>

   <parameter_list> ::=   <parameter_declaration>
                        | <parameter_list>, <parameter_declaration>

   <parameter_declaration> ::= <type-specifier> <identifier>                        

   <type-specifier> ::= "char" | "short" | "int" | "long"

   <block> ::= "{" { <statement> } "}"

   <statement> ::=  <var_declaration>
                    | <labeled_statement>
                    | <assignment_statement>
                    | <return_statement>
                    | <if_statement>
                    | <while_statement>
                    | <for_statement>
                    | <do_while_statement>
                    | <block>
                    | <expression_stmt>
                    | <assert_extension_statement>
                    | <print_extension_statement>

   <labeled_statement> ::= <identifier> : <statement>
                           | "case" <constant-expression> : <statement>
                           | "default" : <statement> 

   <assert_extension_statement> ::= "_assert" "(" <expression> ")" ";"

   <print_extension_statement> ::= "_print" "(" <expression> ")" ";"

   <var-declaration> ::= <type-specifier> <identifier> [ "=" <expression> ] ";"

   <assignment_statement> ::= <assignment_expression> ";";

   <assignment_expression> ::=   <identifier> "=" <expression>
                               | <identifier> '+=' <expression> 
                               | <identifier> '-=' <expression>

   <return_statement>     ::= "return" <expression> ";"

   <if_statement> ::= "if" "(" <expression> ")" <statement> [ "else" <statement> ]

   <while_statement> ::= "while" "(" <expression> ")" <statement> 

   <for_statement> ::= "for" "(" [init_expr] ";" [cond_expr] ";" [update_expr] ")" statement

   <do_while_statement> ::= "do" <statement> "while" "(" <expression> ")" ";"
   
   <expression_stmt> ::= <expression> ";"

   <expression> ::= <logical_or_expr>
   
   <logical_or_expr> ::= <logical_and_expr> { '||' <logical_and_expr> }*

   <logical_and_expr> ::= <equality_expr> { "&&" <equality_expr> }*

   <equality_expr> ::= <relational_expr> [ ("==" | "!=") <relational_expr> ]*

   <relational_expr> ::= <additive_expr> [ ( "<" | "<=" | ">" | ">=") <additive_expr> ]*

   <additive_expr> ::= <term> [ ("+" | "-") <multiplicative_expr> ]*

   <multiplicative_expr>       ::= <cast_expr> [ ("*" | "/" | "%") <cast_expr> ]*

   <cast_expr>        ::= <unary_expr>
                         | "(" <type_specifier> ")" <cast_expr>
   
   <unary_expr> ::= [ "+" | "-" | "!" | "++"" | "--"" ] <unary_expr> | <primary>

   <postfix_expr> ::= <primary> [ "++" | "--" ]?

   <primary>     ::= <int_literal> 
                    | <identifier>
                    | <identifier> "(" <argument-expression-list>* ")"
                    | "(" <expression> ")"

   <argument-expression-list> ::= <assignment-expression>
                                | <argument-expression-list> , <assignment-expression>                    o       

   <identifier>    ::= [a-zA-Z_][a-zA-Z0-9_]*

    <int_literal> ::= [0-9]+
*/

void free_param_info(ParamInfo * param_info) {
    free(param_info->name);
}

CType_list * get_ctype_list(ParamInfo_list * param_list) {
    CType_list * type_list = NULL;
    type_list = malloc(sizeof(CType_list));
    CType_list_init(type_list, free_ctype);
    for (ParamInfo_list_node * n = param_list->head; n != NULL; n = n->next) {
        CType_list_append(type_list, (CType *)n->value->type);
    }
    return type_list;
}
/* Top level parser method */
ASTNode* parse(tokenlist * tokens) {
    ParserContext * parserContext = create_parser_context(tokens);
    ASTNode * node = parse_translation_unit(parserContext);
    free_parser_context(parserContext);
    return node;
}

ASTNode* parse_translation_unit(ParserContext * parserContext) {
    ASTNode_list * functions_list = create_node_list();
    ASTNode_list * globals_list = create_node_list();

    while (!is_current_token(parserContext, TOKEN_EOF)) {
        // currently only supporting functions as topmost. will need to add file level variables.
        ASTNode * external_decl = parse_external_declaration(parserContext);
        if (external_decl->type == AST_VAR_DECL) {
            ASTNode_list_append(globals_list, external_decl);
        } else {
            ASTNode_list_append(functions_list, external_decl);
        }
    }

    return create_translation_unit_node(functions_list, globals_list);
}

ASTNode * parse_external_declaration(ParserContext * parserContext) {

    CType * base_type = parse_type_specifier(parserContext);

    Declarator * declarator = parse_declarator(parserContext, base_type);

    if (declarator->type->kind == CTYPE_FUNCTION) {
        if (is_current_token(parserContext, TOKEN_LBRACE)) {
            return parse_function_definition(parserContext, declarator->name, declarator->type, declarator->param_list);
        }
        if (is_current_token(parserContext, TOKEN_SEMICOLON)) {
            // handle forward function declaration
            expect_token(parserContext, TOKEN_SEMICOLON);
            return create_function_declaration_node(declarator->name, declarator->type, declarator->param_list, NULL, true);
        }
    }
    ASTNode * declaration = parse_declaration_tail(parserContext, declarator->type, declarator->name);
    declaration->var_decl.is_global = true;

    free(declarator->name);
    free(declarator);

    return declaration;

}

Declarator * make_declarator() {
    Declarator * declarator = malloc(sizeof(Declarator));
    declarator->name = NULL;
    declarator->type = NULL;
    declarator->param_list = NULL;
    return declarator;
}

Declarator * parse_declarator(ParserContext * ctx, CType * base_type /*,
    char ** out_name, ASTNode_list ** out_params, CType ** func_type*/) {
    // parse pointer layer

    ASTNode_list * astParam_list = NULL;
    Declarator *declarator = make_declarator();

    while (match_token(ctx, TOKEN_STAR)) {
        base_type = make_pointer_type(base_type);
    }

    // parse identifier
    Token* name_id = expect_token(ctx, TOKEN_IDENTIFIER);
    declarator->name = strdup(name_id->text);
    // if (out_name) {
    //     *out_name = strdup(name_id->text);
    // }

    while (true) {
        if (match_token(ctx, TOKEN_LBRACKET)) {
            // array declarator
            ASTNode * len_node = parse_constant_expression(ctx);
            int len = len_node->int_value;
            expect_token(ctx, TOKEN_RBRACKET);
            base_type = make_array_type(base_type, len);
        }
        else if (is_current_token(ctx, TOKEN_LPAREN)) {
            if (is_next_token(ctx, TOKEN_RPAREN)) {
                advance_parser(ctx);
                advance_parser(ctx);
                base_type = make_function_type(base_type, NULL);
                // if (func_type) {
                //     *func_type = make_function_type(base_type, NULL);
                // }
            }
            else {
                advance_parser(ctx);
                ParamInfo_list * param_types = parse_parameter_type_list(ctx/*, out_params*/);
                expect_token(ctx, TOKEN_RPAREN); //maybe this should be a comma or semicolon ...
                base_type = make_function_type(base_type, get_ctype_list(param_types));
                astParam_list = create_node_list();
                for (ParamInfo_list_node * n = param_types->head; n != NULL; n = n->next) {
                    ASTNode_list_append(astParam_list, n->value->astNode);
                }
                // if (func_type) {
                //     *func_type = make_function_type(base_type, param_types);
                // }
            }
        }
        else {
            break;
        }
    }
                // ASTNode * node = create_var_decl_node(name, base_type, NULL);
                // return node;
    declarator->type = base_type;

    declarator->param_list = astParam_list;
    return declarator;
}

ParamInfo_list * parse_parameter_type_list(ParserContext * ctx/*, ASTNode_list ** out_params*/) {
//    CType_list * type_list = NULL;
    ParamInfo_list * param_list = NULL;
//    if (out_params) *out_params = NULL;
    while (true) {
        CType * base_type = parse_type_specifier(ctx);

//        Token * ident = expect_token(ctx, TOKEN_IDENTIFIER);
//        char * out_name;
        Declarator * declarator = parse_declarator(ctx, base_type /*, &out_name, NULL, NULL*/);

        ASTNode * param = create_var_decl_node(declarator->name, declarator->type, NULL);
//        param->ctype = full_type;
//        param->var_decl.ctype = full_type;

        // if (out_params) {
        //     if (*out_params == NULL) {
        //         *out_params = malloc(sizeof(ASTNode_list));
        //         ASTNode_list_init(*out_params, free_ast);
        //     }
        //     ASTNode_list_append(*out_params, param);
        // }

        // if (type_list == NULL) {
        //     type_list = malloc(sizeof(CType_list));
        //     CType_list_init(type_list, free_ctype);
        // }

        if (param_list == NULL) {
            param_list = malloc(sizeof(ParamInfo_list));
            ParamInfo_list_init(param_list, free_param_info);
        }

//        CType_list_append(type_list,full_type);

        ParamInfo * paramInfo = malloc(sizeof(ParamInfo));
        paramInfo->name = strdup(declarator->name);
        paramInfo->type = declarator->type;
        paramInfo->astNode = param;

        ParamInfo_list_append(param_list, paramInfo);
        if (!match_token(ctx, TOKEN_COMMA)) break;
    }

    return param_list;

}

ASTNode_list * parse_param_list(ParserContext * parserContext) {
    ASTNode_list * param_list = create_node_list();

    do {
        CType * ctype = parse_type_specifier(parserContext);
        Token * param_name = expect_token(parserContext, TOKEN_IDENTIFIER);
        ASTNode * param = create_var_decl_node(param_name->text, ctype, NULL);
        param->var_decl.is_param = true;
        ASTNode_list_append(param_list, param);
    } while (match_token(parserContext, TOKEN_COMMA));
    return param_list;
}

ASTNode_list * parse_argument_expression_list(ParserContext * parserContext) {

    ASTNode_list * arg_list = create_node_list();
    
    do {
        ASTNode * expression = parse_expression(parserContext);
        ASTNode_list_append(arg_list, expression);
    } while (match_token(parserContext, TOKEN_COMMA));
    return arg_list;
}

CType * parse_type_specifier(ParserContext * ctx) {
    TokenType type = peek(ctx)->type;
    switch (type) {
        case TOKEN_INT: advance_parser(ctx); return &CTYPE_INT_T;
        case TOKEN_CHAR: advance_parser(ctx); return &CTYPE_CHAR_T;
        case TOKEN_SHORT: advance_parser(ctx); return &CTYPE_SHORT_T;
        case TOKEN_LONG: advance_parser(ctx); return &CTYPE_LONG_T;
        default: error("Invalid Type - %s", token_type_name(type)); return NULL;
    }
}


ASTNode * parse_function_definition(ParserContext * parserContext, char * name, CType * full_type, ASTNode_list * params) {

//    char * name = NULL;
//    ASTNode_list * params = NULL;
//    CType * func_type = NULL;
//    CType * full_type = parse_declarator(parserContext, base_type, &name/*, &params, &func_type*/);

    ASTNode * body = NULL;

    body = parse_block(parserContext);

    ASTNode * func = create_function_declaration_node(name, full_type, params
        ,body, false);

    return func;

}

ASTNode*  parse_declaration_tail(ParserContext * parserContext, CType * ctype, char * id) {
//    CType * ctype = parse_ctype(parserContext);
//    Token * name = expect_token(parserContext, TOKEN_IDENTIFIER);
    ASTNode * initializer_expr = NULL;
    if (is_current_token(parserContext, TOKEN_ASSIGN)) {
        advance_parser(parserContext);
        if (is_current_token(parserContext, TOKEN_LBRACE)) {
            initializer_expr = parse_initializer_list(parserContext);
        }
        else {
            initializer_expr = parse_expression(parserContext);
        }
    }
    expect_token(parserContext, TOKEN_SEMICOLON);

    ASTNode * node = create_var_decl_node(id, ctype, initializer_expr);
//    node->var_decl.is_global = true;

    return node;
}

// ASTNode * parse_function(ParserContext* parserContext) {
//  //   expect_token(parserContext, TOKEN_INT);
//     Token* returnCType = expect_ctype_token(parserContext);
//     Token* name = expect_token(parserContext, TOKEN_IDENTIFIER);
//     expect_token(parserContext, TOKEN_LPAREN);
//     ASTNode_list *param_list = NULL;
//
//     if (!is_current_token(parserContext, TOKEN_RPAREN)) {
//         param_list = parse_param_list(parserContext);
//     }
//
//     expect_token(parserContext, TOKEN_RPAREN);
//     bool declaration_only;
//     ASTNode * function_block = NULL;
//     if (is_current_token(parserContext, TOKEN_LBRACE)) {
//         function_block = parse_block(parserContext);
//         declaration_only = false;
//     }
//     else {
//         expect_token(parserContext, TOKEN_SEMICOLON);
//         declaration_only = true;
//     }
//
//     ASTNode * func = create_function_declaration_node(name->text, get_ctype_from_token(returnCType),
//         param_list, function_block, declaration_only);
//
//     return func;
// }

/////////////////////////////////////////////////////////
//// STATEMENTS
/////////////////////////////////////////////////////////
ASTNode * parse_assert_extension_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_ASSERT_EXTENSION);
    expect_token(parserContext, TOKEN_LPAREN);

    ASTNode * expr = parse_expression(parserContext);

    expect_token(parserContext, TOKEN_RPAREN);
    expect_token(parserContext, TOKEN_SEMICOLON);

    return create_assert_extension_node(expr);
}

ASTNode * parse_print_extension_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_PRINT_EXTENSION);
    expect_token(parserContext, TOKEN_LPAREN);

    ASTNode * expr = parse_expression(parserContext);

    expect_token(parserContext, TOKEN_RPAREN);
    expect_token(parserContext, TOKEN_SEMICOLON);

    return create_print_extension_node(expr);
}

ASTNode * parse_block(ParserContext* parserContext) {
    expect_token(parserContext, TOKEN_LBRACE);

    ASTNode_list * statements = create_node_list();

    while(!is_current_token(parserContext, TOKEN_RBRACE)) {
        ASTNode * statement = parse_statement(parserContext);
        ASTNode_list_append(statements, statement);
    }
        
    expect_token(parserContext, TOKEN_RBRACE);

    return create_block_node(statements);
}

ASTNode * parse_label_statement(ParserContext* parserContext) {
    Token * labelToken = expect_token(parserContext, TOKEN_IDENTIFIER);
    const char * label = labelToken->text;
    
    expect_token(parserContext, TOKEN_COLON);
    ASTNode * stmt = parse_statement(parserContext);

    return create_ast_labeled_statement_node(label, stmt);
}


ASTNode * parse_case_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_CASE);
    ASTNode * constantExpression = parse_constant_expression(parserContext);
    expect_token(parserContext, TOKEN_COLON);
    ASTNode * stmt = parse_statement(parserContext);

    return create_ast_case_statement_node(constantExpression, stmt);
}

ASTNode * parse_default_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_DEFAULT);
    expect_token(parserContext, TOKEN_COLON);
    ASTNode * stmt = parse_statement(parserContext);

    return create_ast_default_statement_node(stmt);
}

ASTNode * parse_goto_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_GOTO);
    Token * labelToken = expect_token(parserContext, TOKEN_IDENTIFIER);
    const char * label = labelToken->text;
    expect_token(parserContext, TOKEN_SEMICOLON);

    return create_goto_statement(label);
}

ASTNode * parse_do_while_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_DO);
    ASTNode * stmt = parse_statement(parserContext);
    expect_token(parserContext, TOKEN_WHILE);
    expect_token(parserContext, TOKEN_LPAREN);
    ASTNode * expr = parse_expression(parserContext);
    expect_token(parserContext, TOKEN_RPAREN);
    expect_token(parserContext, TOKEN_SEMICOLON);

    return create_do_while_statement(stmt, expr);
}

ASTNode * parse_switch_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_SWITCH);
    expect_token(parserContext, TOKEN_LPAREN);
    ASTNode * expr = parse_expression(parserContext);
    expect_token(parserContext, TOKEN_RPAREN);
    ASTNode * stmt = parse_statement(parserContext);

    return create_switch_statement(expr, stmt);
}

ASTNode * parse_break_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_BREAK);
    expect_token(parserContext, TOKEN_SEMICOLON);

    return create_break_statement_node();
}

ASTNode * parse_continue_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_CONTINUE);
    expect_token(parserContext, TOKEN_SEMICOLON);

    return create_continue_statement_node();
}


ASTNode * parse_statement(ParserContext* parserContext) {
    if (is_current_token_a_ctype(parserContext)) {
        return parse_local_declaration(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_RETURN)) {
        return parse_return_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_LBRACE)) {
        return parse_block(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_IF)) {
        return parse_if_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_WHILE)) {
        return parse_while_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_DO)) {
        return parse_do_while_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_FOR)) {
        return parse_for_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_ASSERT_EXTENSION)) {
        return parse_assert_extension_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_PRINT_EXTENSION)) {
        return parse_print_extension_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_IDENTIFIER) && is_next_token(parserContext, TOKEN_COLON)) {
        return parse_label_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_CASE)) {
        return parse_case_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_DEFAULT)) {
        return parse_default_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_GOTO)) {
        return parse_goto_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_SWITCH)) {
        return parse_switch_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_BREAK)) {
        return parse_break_statement(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_CONTINUE)) {
        return parse_continue_statement(parserContext);
    }
    return parse_expression_statement(parserContext);
}

ASTNode*  parse_local_declaration(ParserContext * parserContext) {
    CType * base_type = parse_type_specifier(parserContext);

//    char * name = NULL;
    Declarator * declarator = parse_declarator(parserContext, base_type/* , &name, NULL, NULL*/);

    return parse_declaration_tail(parserContext, declarator->type, declarator->name);
//    Token * name = expect_token(parserContext, TOKEN_IDENTIFIER);
    // ASTNode * init_expr = NULL;
    // if (is_current_token(parserContext, TOKEN_ASSIGN)) {
    //     advance_parser(parserContext);
    //
    //     if (is_current_token(parserContext, TOKEN_LBRACE)) {
    //         init_expr = parse_initializer_list(parserContext);
    //     }
    //     else {
    //         init_expr = parse_expression(parserContext);
    //     }
    // }
    // expect_token(parserContext, TOKEN_SEMICOLON);
    //
    // ASTNode * node = create_var_decl_node(name, full_type, init_expr);
    //
    // return node;
}

ASTNode * parse_initializer_list(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_LBRACE);

    ASTNode_list * items = create_node_list();

    while (!is_current_token(parserContext, TOKEN_RBRACE)) {
        ASTNode * item = NULL;
        if (is_current_token(parserContext, TOKEN_LBRACE)) {
            item = parse_initializer_list(parserContext);
        }
        else {
            item = parse_expression(parserContext);
        }
        ASTNode_list_append(items, item);

        if (is_current_token(parserContext, TOKEN_COMMA)) {
            advance_parser(parserContext);
        }
        else {
            break;
        }
    }

    expect_token(parserContext, TOKEN_RBRACE);

    return create_initializer_list(items);
}

ASTNode * parse_if_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_IF);
    expect_token(parserContext, TOKEN_LPAREN);
    ASTNode * condExpression = parse_expression(parserContext);
    expect_token(parserContext, TOKEN_RPAREN);
    ASTNode * then_statement = parse_statement(parserContext);
    ASTNode * else_statement = NULL;
    if (is_current_token(parserContext, TOKEN_ELSE)) {
        advance_parser(parserContext);
        else_statement = parse_statement(parserContext);
    }

    return create_if_else_statement_node(condExpression, then_statement, else_statement);

}

ASTNode * parse_while_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_WHILE);
    expect_token(parserContext, TOKEN_LPAREN);
    ASTNode * condExpression = parse_expression(parserContext);
    expect_token(parserContext, TOKEN_RPAREN);
    ASTNode * body_statement = parse_statement(parserContext);

    return create_while_statement_node(condExpression, body_statement);
}

ASTNode * parse_for_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_FOR);
    expect_token(parserContext, TOKEN_LPAREN);

    ASTNode * init_expr = NULL;
    ASTNode * cond_expr = NULL;
    ASTNode * update_expr = NULL;

    if (!is_current_token(parserContext, TOKEN_SEMICOLON)) {
        if (is_current_token(parserContext, TOKEN_INT)) {
            init_expr = parse_local_declaration(parserContext);
        }
        else if (is_current_token(parserContext, TOKEN_IDENTIFIER)) {
            init_expr = parse_assignment_expression(parserContext);
            expect_token(parserContext, TOKEN_SEMICOLON);
        }
        else {
            init_expr = parse_expression_statement(parserContext);
        }
    }
    else {
        expect_token(parserContext, TOKEN_SEMICOLON);
    }

    if (!is_current_token(parserContext, TOKEN_SEMICOLON)) {
//        cond_expr = parse_expression_statement(parserContext);
        cond_expr = parse_expression(parserContext);
        expect_token(parserContext, TOKEN_SEMICOLON);
    }
    else {
        expect_token(parserContext, TOKEN_SEMICOLON);
    }

    if (!is_current_token(parserContext, TOKEN_RPAREN)) {
        if (is_current_token(parserContext, TOKEN_IDENTIFIER) && 
            (is_next_token(parserContext, TOKEN_ASSIGN) || is_next_token(parserContext, TOKEN_PLUS_EQUAL) || is_next_token(parserContext, TOKEN_MINUS_EQUAL))) {
            update_expr = parse_assignment_expression(parserContext);
        }
        else {
            update_expr = parse_expression(parserContext);
        }
    }
    expect_token(parserContext, TOKEN_RPAREN);

    ASTNode * body = parse_statement(parserContext);

    ASTNode * node = create_for_statement_node(
        init_expr, cond_expr, update_expr, body);

    return node;
}

ASTNode * parse_expression_statement(ParserContext * parserContext) {
    ASTNode * expr = parse_expression(parserContext);
    expect_token(parserContext, TOKEN_SEMICOLON);
    return create_expression_statement_node(expr);
}

ASTNode * parse_return_statement(ParserContext* parserContext) {
    expect_token(parserContext, TOKEN_RETURN);
    ASTNode * expr = parse_expression(parserContext);
    expect_token(parserContext, TOKEN_SEMICOLON);

    return create_return_statement_node(expr);
}

////////////////////////////////////////////////////////////
////  EXPRESSIONS
////////////////////////////////////////////////////////////
ASTNode * parse_expression(ParserContext * parserContext) {
    return parse_assignment_expression(parserContext);
}

ASTNode * parse_assignment_expression(ParserContext * parserContext) {

    ASTNode * lhs = parse_logical_or(parserContext);

    if (is_current_token(parserContext, TOKEN_ASSIGN)) {
        expect_token(parserContext, TOKEN_ASSIGN);
        ASTNode * rhs = parse_assignment_expression(parserContext);
        return create_binary_node(lhs, BINOP_ASSIGNMENT, rhs);
    }
    if (is_current_token(parserContext, TOKEN_PLUS_EQUAL)) {
        expect_token(parserContext, TOKEN_PLUS_EQUAL);
        ASTNode * rhs = parse_expression(parserContext);
        return create_binary_node(lhs, BINOP_COMPOUND_ADD_ASSIGN, rhs);
    }
    if (is_current_token(parserContext, TOKEN_MINUS_EQUAL)) {
        expect_token(parserContext, TOKEN_MINUS_EQUAL);
        ASTNode * rhs = parse_expression(parserContext);
        return create_binary_node(lhs, BINOP_COMPOUND_SUB_ASSIGN, rhs);
    }
    return lhs;
}

ASTNode * parse_logical_or(ParserContext * parserContext) {
    ASTNode * lhs = parse_logical_and(parserContext);

    while (match_token(parserContext, TOKEN_LOGICAL_OR)) {
        ASTNode * rhs = parse_logical_and(parserContext);
        ASTNode * node = create_binary_node(lhs, BINOP_LOGICAL_OR, rhs);
        lhs = node;
    }
    return lhs;
}

ASTNode * parse_logical_and(ParserContext * parserContext) {
    ASTNode * lhs = parse_equality_expression(parserContext);

    while (match_token(parserContext, TOKEN_LOGICAL_AND)) {
        ASTNode * rhs = parse_equality_expression(parserContext);
        ASTNode * node = create_binary_node(lhs, BINOP_LOGICAL_AND, rhs);
        lhs = node;
    }
    return lhs;
}

ASTNode * parse_equality_expression(ParserContext * parserContext) {
    ASTNode * root = parse_relational_expression(parserContext);

    while(is_current_token(parserContext, TOKEN_EQ) || is_current_token(parserContext, TOKEN_NEQ)) {
        ASTNode * lhs = root;
        Token * op = peek(parserContext);
        BinaryOperator binop = (op->type == TOKEN_EQ) ? BINOP_EQ : BINOP_NE;
        advance_parser(parserContext);
        ASTNode * rhs = parse_relational_expression(parserContext);
        root = create_binary_node(lhs, binop, rhs);
    }

    return root;
}

ASTNode * parse_relational_expression(ParserContext * parserContext) {
    ASTNode * root = parse_additive_expression(parserContext);
    
    while(is_current_token(parserContext, TOKEN_GT) || is_current_token(parserContext, TOKEN_GE) 
            || is_current_token(parserContext, TOKEN_LT) || is_current_token(parserContext, TOKEN_LE)) {
        ASTNode * lhs = root;
        Token * op = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * rhs = parse_additive_expression(parserContext);
        root = create_binary_node(lhs, get_binary_operator_from_tok(op), rhs);
    }
    return root;
}

ASTNode * parse_additive_expression(ParserContext * parserContext) {
    ASTNode * root = parse_multiplicative_expression(parserContext);

    while(is_current_token(parserContext, TOKEN_PLUS) || is_current_token(parserContext, TOKEN_MINUS)) {
        ASTNode * lhs = root;
        Token * op = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * rhs = parse_multiplicative_expression(parserContext);
        root = create_binary_node(lhs, get_binary_operator_from_tok(op), rhs);
    }
    return root;
}


ASTNode * parse_multiplicative_expression(ParserContext * parserContext) {
    ASTNode * root = parse_cast_expression(parserContext);

    while(is_current_token(parserContext, TOKEN_STAR) || is_current_token(parserContext,TOKEN_DIV) || is_current_token(parserContext, TOKEN_PERCENT)) {
        ASTNode * lhs = root;
        Token * op = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * rhs = parse_cast_expression(parserContext);
        root = create_binary_node(lhs, get_binary_operator_from_tok(op), rhs);
    }

    return root;
}

ASTNode * parse_cast_expression(ParserContext * parserContext) {
    if (is_current_token(parserContext, TOKEN_LPAREN) && is_next_token_a_ctype(parserContext)) {
        expect_token(parserContext, TOKEN_LPAREN);
        CType * target_type = parse_type_specifier(parserContext);
        expect_token(parserContext, TOKEN_RPAREN);
        ASTNode * expr = parse_cast_expression(parserContext);
        ASTNode * node = create_cast_expr_node(target_type, expr);
        return node;
    }

    return parse_unary_expression(parserContext);
}

ASTNode * parse_unary_expression(ParserContext * parserContext) {
    if (is_current_token(parserContext, TOKEN_PLUS)) {
        expect_token(parserContext, TOKEN_PLUS);
        ASTNode * operand = parse_cast_expression(parserContext);
        return create_unary_node(UNARY_PLUS, operand);
    }
    if (is_current_token(parserContext, TOKEN_MINUS)) {
        expect_token(parserContext, TOKEN_MINUS);
        ASTNode * operand = parse_cast_expression(parserContext);
        return create_unary_node(UNARY_NEGATE, operand);
    }
    if (is_current_token(parserContext, TOKEN_BANG)) {
        expect_token(parserContext, TOKEN_BANG);
        ASTNode * operand = parse_cast_expression(parserContext);
        return create_unary_node(UNARY_NOT, operand);
    }
    if (is_current_token(parserContext, TOKEN_STAR)) {
        expect_token(parserContext, TOKEN_STAR);
        ASTNode * operand = parse_cast_expression(parserContext);
        return create_unary_node(UNARY_DEREF, operand);
    }
    if (is_current_token(parserContext, TOKEN_AMPERSAND)) {
        expect_token(parserContext, TOKEN_AMPERSAND);
        ASTNode * operand = parse_cast_expression(parserContext);
        return create_unary_node(UNARY_ADDRESS, operand);
    }
    if (is_current_token(parserContext, TOKEN_INCREMENT)) {
        expect_token(parserContext, TOKEN_INCREMENT);
        ASTNode * operand = parse_unary_expression(parserContext);
        return create_unary_node(UNARY_PRE_INC, operand);
    }
    if (is_current_token(parserContext, TOKEN_DECREMENT)) {
        expect_token(parserContext, TOKEN_DECREMENT);
        ASTNode * operand = parse_unary_expression(parserContext);
        return create_unary_node(UNARY_PRE_DEC, operand);
    }
    return parse_postfix_expression(parserContext);

}


ASTNode * parse_postfix_expression(ParserContext * parserContext) {
    ASTNode * primary = parse_primary(parserContext);

    if (is_current_token(parserContext, TOKEN_INCREMENT)) {
        expect_token(parserContext, TOKEN_INCREMENT);

        return create_unary_node(UNARY_POST_INC, primary);
    }
    if (is_current_token(parserContext, TOKEN_DECREMENT)) {
        expect_token(parserContext, TOKEN_DECREMENT);

        return create_unary_node(UNARY_POST_DEC, primary);
    }
    if (is_current_token(parserContext, TOKEN_LPAREN)) {
        advance_parser(parserContext);
        ASTNode_list * argument_expression_list = NULL;
        if (!is_current_token(parserContext, TOKEN_RPAREN)) {
            argument_expression_list = parse_argument_expression_list(parserContext);
            expect_token(parserContext, TOKEN_RPAREN);
        }
        else {
            expect_token(parserContext, TOKEN_RPAREN);
        }
        ASTNode * node = create_function_call_node(primary->var_ref.name, argument_expression_list);
        return node;
    }
    if (is_current_token(parserContext, TOKEN_LBRACKET)) {
        while (true) {
            if (match_token(parserContext, TOKEN_LBRACKET)) {
                ASTNode* index = parse_expression(parserContext);
                expect_token(parserContext, TOKEN_RBRACKET);

                ASTNode * array_node = create_array_access_node(primary, index);
                primary = array_node;
            } else {
                break;
            }
        }
    }

    return primary;
}

ASTNode * parse_constant_expression(ParserContext * parserContext) {
    ASTNode * node = parse_primary(parserContext);
    if (node->type == AST_INT_LITERAL) {
        return node;
    }
    error("expected constant expression\n");
    return NULL;
}

ASTNode * parse_primary(ParserContext * parserContext) {

    // handle integer literals
    if (is_current_token(parserContext, TOKEN_INT_LITERAL)) {
        Token * tok = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * node = create_int_literal_node(tok->int_value);
        return node;
    }
    // handle paren blocks
    if (is_current_token(parserContext, TOKEN_LPAREN)) {
        expect_token(parserContext, TOKEN_LPAREN);
        ASTNode * node = parse_expression(parserContext);
        expect_token(parserContext, TOKEN_RPAREN);
        return node;
    }
    // handle identifier related
    if (is_current_token(parserContext, TOKEN_IDENTIFIER)) {
        Token * tok = peek(parserContext);
        advance_parser(parserContext);
        return create_var_ref_node(tok->text);
    }
    error("Unhandled token error, %s", get_current_token_type_name(parserContext));
    return NULL;

}

