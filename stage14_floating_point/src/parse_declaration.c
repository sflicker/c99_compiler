//
// Created by scott on 8/16/25.
//

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
#include "c_type_printer.h"
#include "parse_expression.h"
#include "parse_declaration.h"

CType * parse_type_specifier(ParserContext * ctx) {
    TokenType type = peek(ctx)->type;
    switch (type) {
        case TOKEN_INT: advance_parser(ctx); return &CTYPE_INT_T;
        case TOKEN_CHAR: advance_parser(ctx); return &CTYPE_CHAR_T;
        case TOKEN_SHORT: advance_parser(ctx); return &CTYPE_SHORT_T;
        case TOKEN_LONG: advance_parser(ctx); return &CTYPE_LONG_T;
        case TOKEN_FLOAT: advance_parser(ctx); return &CTYPE_FLOAT_T;
        case TOKEN_DOUBLE: advance_parser(ctx); return &CTYPE_DOUBLE_T;
        default: error("Invalid Type - %s", token_type_name(type)); return NULL;
    }
}

ASTNode * parse_external_declaration(ParserContext * parserContext) {

    Declarator *declarator = make_declarator();
    declarator->type = parse_type_specifier(parserContext);
    print_c_type(declarator->type, 0);

    //    declarator->type = base_type;

    declarator = parse_declarator(parserContext, declarator);
    const char * name = declarator->name;
    ASTNode_list * param_list = declarator->param_list;

    if (declarator->type->kind == CTYPE_FUNCTION) {
        if (is_current_token(parserContext, TOKEN_LBRACE)) {
            return parse_function_definition(parserContext, name, declarator->type, param_list );
        }
        if (is_current_token(parserContext, TOKEN_SEMICOLON)) {
            // handle forward function declaration
            expect_token(parserContext, TOKEN_SEMICOLON);
            return create_function_declaration_node(name, declarator->type, param_list, NULL, true);
        }
    }
    ASTNode * declaration = parse_declaration_tail(parserContext, declarator->type, name);
    declaration->var_decl.is_global = true;

    //    free(name);

    return declaration;
}

Declarator * parse_declarator(ParserContext * ctx, Declarator * declarator /*,
    char ** out_name, ASTNode_list ** out_params, CType ** func_type*/) {
    // parse pointer layer

    //    ASTNode_list * astParam_list = NULL;
    // Declarator *declarator = make_declarator();
    // declarator->type = base_type;

    while (match_token(ctx, TOKEN_STAR)) {
        declarator->type = make_pointer_type(declarator->type);
    }

    return parse_direct_declarator(ctx, declarator);
}

Declarator * parse_direct_declarator(ParserContext * ctx, Declarator * declarator) {
    if (match_token(ctx, TOKEN_LPAREN)) {
        declarator = parse_declarator(ctx, declarator);
        expect_token(ctx, TOKEN_RPAREN);
        declarator = parse_postfix_declarator(ctx, declarator);
    } else {
        Token* tok_name = expect_token(ctx, TOKEN_IDENTIFIER);
        char * name = strdup(tok_name->text);
        declarator->name = strdup(name);
        //        set_current_decl_name(ctx, name);
        declarator = parse_postfix_declarator(ctx, declarator);
    }
    return declarator;
}

typedef struct ArraySizeList {
    int count;
    int sizes[10];
} ArraySizeList;

Declarator * parse_postfix_declarator(ParserContext * ctx, Declarator * declarator) {
    ASTNode_list * astParam_list = NULL;
    ArraySizeList array_sizes = {0};

    while (true) {
        if (match_token(ctx, TOKEN_LBRACKET)) {
            // array declarator
            ASTNode * len_node = parse_constant_expression(ctx);
            int len = len_node->int_value;
            expect_token(ctx, TOKEN_RBRACKET);
            //            base_type = make_array_type(base_type, len);
            array_sizes.sizes[array_sizes.count++] = len;

        }
        else if (is_current_token(ctx, TOKEN_LPAREN)) {
            if (is_next_token(ctx, TOKEN_RPAREN)) {
                advance_parser(ctx);
                advance_parser(ctx);
                declarator->type = make_function_type(declarator->type, NULL);
                // if (func_type) {
                //     *func_type = make_function_type(base_type, NULL);
                // }
            }
            else {
                advance_parser(ctx);
                ParamInfo_list * param_types = parse_parameter_type_list(ctx/*, out_params*/);
                expect_token(ctx, TOKEN_RPAREN); //maybe this should be a comma or semicolon ...
                declarator->type = make_function_type(declarator->type, get_ctype_list(param_types));
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
    declarator->param_list = astParam_list;
//    set_current_decl_param_list(ctx, astParam_list);

    // reverse the array dimensions
    for (int i = array_sizes.count - 1; i >= 0; --i) {
        declarator->type = make_array_type(declarator->type, array_sizes.sizes[i]);
    }

    return declarator;
}

ASTNode*  parse_declaration_tail(ParserContext * parserContext, CType * ctype, const char * id) {
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

    // TODO. FIX - THIS IS BROKEN IF COMMA IS NEXT INSTEAD OF SEMICOLON
    expect_token(parserContext, TOKEN_SEMICOLON);

    ASTNode * node = create_var_decl_node(id, ctype, initializer_expr);
    //    node->var_decl.is_global = true;

    return node;
}
