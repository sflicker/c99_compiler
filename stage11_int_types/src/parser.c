#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "list_util.h"
#include "token.h"
#include "ast_list.h"
#include "ast.h"

#include "parser.h"
#include "parser_util.h"
#include "parser_context.h"
#include "ctype.h"

/* 
   Simple C compiler example 

   Simplifed grammar

   <translation-unit>       ::= <external-declaration>

   <external-declaration>   ::= <function>

   <function> ::= "int" <identifier> "(" <parameter_list>* ")" 
                [ <block> | ";" ]

   <parameter_list> ::=   <parameter_declaration>
                        | <parameter_list>, <parameter_declaration>

   <parameter_declaration> ::= <type-specifier> <identifier>                        

   <type-specifier> ::= "char" | "short" | "int" | "long"

   <block> ::= "{" { <statement> } "}"

   <statement> ::=  <var-declaration>
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

   <additive_expr> ::= <term> [ ("+" | "-") <term> ]*

   <term>       ::= <unary_expr> [ ("*" | "/") <unary_expr> ]*
   
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



//Token* expect_token(ParserContext * parserContext, TokenType expected);

//Token* peek(ParserContext * parserContext);
//Token* advance_parser(ParserContext * parserContext);
//bool match_token(ParserContext* p, TokenType type);


void nop_free_astnode(ASTNode * node) {

}

ASTNode* parse(tokenlist * tokens) {
    ParserContext * parserContext = create_parser_context(tokens);
    ASTNode * node = parse_translation_unit(parserContext);
    free_parser_context(parserContext);
    return node;
}

ASTNode* parse_translation_unit(ParserContext * parserContext) {
    ASTNode_list * functions = malloc(sizeof(ASTNode_list));
    ASTNode_list_init(functions, free_astnode);

    while (!is_current_token(parserContext, TOKEN_EOF)) {
        // currently only supporting functions as topmost. will need to add file level variables.
        ASTNode * function = parse_external_declaration(parserContext);
        ASTNode_list_append(functions, function);
    }

    return create_translation_unit_node(functions);
}

CType * parse_ctype(ParserContext * ctx) {
    switch (peek(ctx)->type) {
        case TOKEN_INT: advance_parser(ctx); return &CTYPE_INT_T;
        case TOKEN_CHAR: advance_parser(ctx); return &CTYPE_CHAR_T;
        case TOKEN_SHORT: advance_parser(ctx); return &CTYPE_SHORT_T;
        case TOKEN_LONG: advance_parser(ctx); return &CTYPE_LONG_T;
        default: error("Invalid Type"); return NULL;
    }
}


ASTNode * parse_external_declaration(ParserContext * parserContext) {
        // expect_token(parserContext, TOKEN_INT);
        // Token* name = expect_token(parserContext, TOKEN_IDENTIFIER);

        return parse_function(parserContext);
}

ASTNode_list * parse_param_list(ParserContext * parserContext) {
    ASTNode_list * param_list = NULL;

    do {
        CType * ctype = parse_ctype(parserContext);
        Token * param_name = expect_token(parserContext, TOKEN_IDENTIFIER);
        ASTNode * param = create_var_decl_node(param_name->text, ctype, NULL);
        // ASTNode * param = malloc(sizeof(ASTNode));
        // param->type = AST_VAR_DECL;
        // param->var_decl.name = strdup(param_name->text);
        // param->var_decl.var_ctype = ctype;
        // param->var_decl.init_expr = NULL;
//        param->var_decl.addr.kind = ADDR_UNASSIGNED;
        if (param_list == NULL) {
            param_list = malloc(sizeof(ASTNode_list));
            ASTNode_list_init(param_list, free_astnode);
            ASTNode_list_append(param_list, param);
 //           param_list = create_node_list(param);
//            add_node_list(param_list, param);
        }
        else {
            ASTNode_list_append(param_list, param);
//            add_node_list(param_list, param);
        }

    } while (match_token(parserContext, TOKEN_COMMA));
    // ASTNode * ast_param_list = malloc(sizeof(ASTNode));
    // ast_param_list->type = AST_PARAM_LIST;
    // ast_param_list->param_list.node_list = param_list;
    // return ast_param_list;
    return param_list;
}



ASTNode_list * parse_argument_expression_list(ParserContext * parserContext) {

    ASTNode_list * arg_list = malloc(sizeof(ASTNode_list));
    ASTNode_list_init(arg_list, free_astnode);
    
    do {
        ASTNode * expression = parse_expression(parserContext);
        ASTNode_list_append(arg_list, expression);
//        if (arg_list == NULL) {
            //arg_list = create_node_list(expression);
//            add_node_list(arg_list, expression);
//        }
//        else {
//            add_node_list(arg_list, expression);
//        }

    } while (match_token(parserContext, TOKEN_COMMA));
    return arg_list;
}

CType * get_ctype(Token* token) {
    switch (token->type) {
        case TOKEN_INT: 
            return &CTYPE_INT_T;
            break;
        case TOKEN_CHAR:
            return &CTYPE_CHAR_T;
            break;
        case TOKEN_SHORT:
            return &CTYPE_SHORT_T;
            break;
        case TOKEN_LONG:
            return &CTYPE_LONG_T;
            break;
        default:
            error("Invalid TYPE Token\n");
            break;
    }
    return NULL;
}

ASTNode * parse_function(ParserContext* parserContext) {
 //   expect_token(parserContext, TOKEN_INT);
    Token* returnCType = expect_ctype_token(parserContext);
    Token* name = expect_token(parserContext, TOKEN_IDENTIFIER);
    expect_token(parserContext, TOKEN_LPAREN);
    ASTNode_list *param_list = NULL;

    if (!is_current_token(parserContext, TOKEN_RPAREN)) {
        param_list = parse_param_list(parserContext);
    }

    expect_token(parserContext, TOKEN_RPAREN);
    bool declaration_only;
    ASTNode * function_block = NULL;
    if (is_current_token(parserContext, TOKEN_LBRACE)) {
        function_block = parse_block(parserContext);
        declaration_only = false;
    }
    else {
        expect_token(parserContext, TOKEN_SEMICOLON);
        declaration_only = true;
    }

    ASTNode * func = malloc(sizeof(ASTNode));
    func->type = AST_FUNCTION_DECL;
    func->function_decl.name = strdup(name->text);
    func->ctype = get_ctype(returnCType);
    func->function_decl.body = function_block;
    func->function_decl.param_list = param_list;
    func->function_decl.declaration_only = declaration_only;
    return func;
}

ASTNode * parse_assert_extension_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_ASSERT_EXTENSION);
    expect_token(parserContext, TOKEN_LPAREN);

    ASTNode * expr = parse_expression(parserContext);

    expect_token(parserContext, TOKEN_RPAREN);
    expect_token(parserContext, TOKEN_SEMICOLON);

    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_ASSERT_EXTENSION_STATEMENT;
    node->expr_stmt.expr = expr;
    return node;
}

ASTNode * parse_print_extension_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_PRINT_EXTENSION);
    expect_token(parserContext, TOKEN_LPAREN);

    ASTNode * expr = parse_expression(parserContext);

    expect_token(parserContext, TOKEN_RPAREN);
    expect_token(parserContext, TOKEN_SEMICOLON);

    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_PRINT_EXTENSION_STATEMENT;
    node->expr_stmt.expr = expr;
    return node;
}

ASTNode * parse_block(ParserContext* parserContext) {
    expect_token(parserContext, TOKEN_LBRACE);

    ASTNode * blockNode = malloc(sizeof(ASTNode));
    blockNode->type = AST_BLOCK;
    blockNode->block.statements = malloc(sizeof(ASTNode_list));
    ASTNode_list_init(blockNode->block.statements, free_astnode);
//    blockNode->block.count = 0;
//    blockNode->block.capacity = 4;
//    blockNode->block.statements = malloc(sizeof(ASTNode*) * blockNode->block.capacity);

    while(!is_current_token(parserContext, TOKEN_RBRACE)) {
        ASTNode * statement = parse_statement(parserContext);
        ASTNode_list_append(blockNode->block.statements, statement);
    //     if (blockNode->block.count >= blockNode->block.capacity) {
    //         blockNode->block.capacity *= 2;
    //         blockNode->block.statements = realloc(
    //             blockNode->block.statements, sizeof(ASTNode*) * blockNode->block.capacity);
    //     }
    //     blockNode->block.statements[blockNode->block.count++] = statement;
    }
        
    expect_token(parserContext, TOKEN_RBRACE);

    return blockNode;
}

ASTNode * parse_label_statement(ParserContext* parserContext) {
    Token * labelToken = expect_token(parserContext, TOKEN_IDENTIFIER);
    const char * label = labelToken->text;
    
    expect_token(parserContext, TOKEN_COLON);
    ASTNode * stmt = parse_statement(parserContext);

    return create_ast_labeled_statement_node(label, stmt);
}

ASTNode * parse_constant_expression(ParserContext * parserContext) {
    ASTNode * node = parse_primary(parserContext);
    if (node->type == AST_INT_LITERAL) {
        return node;
    }
    error("expected constant expression\n");
    return NULL;
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
        return parse_var_declaration(parserContext);
    }
    // if (is_current_token(parserContext, TOKEN_IDENTIFIER) && is_next_token_assignment(parserContext)){
    //     return parse_assignment_statement(parserContext);
    // }
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
            init_expr = parse_var_declaration(parserContext);
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
        cond_expr = parse_expression_statement(parserContext);
    }
    else {
        expect_token(parserContext, TOKEN_SEMICOLON);
    }

    // if (!is_current_token(parserContext, TOKEN_SEMICOLON)) {
    //     init_expr = parse_expression(parserContext);
    // }
    // expect_token(parserContext, TOKEN_SEMICOLON);

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

    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_EXPRESSION_STMT;
    node->expr_stmt.expr = expr;
    return node;
}

ASTNode * parse_return_statement(ParserContext* parserContext) {
    expect_token(parserContext, TOKEN_RETURN);
    ASTNode * expr = parse_expression(parserContext);
    expect_token(parserContext, TOKEN_SEMICOLON);

    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_RETURN_STMT;
    node->return_stmt.expr = expr;
    return node;
}


ASTNode * parse_expression(ParserContext * parserContext) {
//    return parse_logical_or(parserContext);
    return parse_assignment_expression(parserContext);
}

ASTNode * parse_logical_or(ParserContext * parserContext) {
    ASTNode * lhs = parse_logical_and(parserContext);

    while (match_token(parserContext, TOKEN_LOGICAL_OR)) {
        ASTNode * rhs = parse_logical_and(parserContext);
        ASTNode * node = malloc(sizeof(ASTNode));

        node->type = AST_BINARY_EXPR;
        node->binary.op = BINOP_LOGICAL_OR;
        node->binary.lhs = lhs;
        node->binary.rhs = rhs;
        lhs = node;
    }
    return lhs;
}

ASTNode * parse_logical_and(ParserContext * parserContext) {
    ASTNode * lhs = parse_equality_expression(parserContext);

    while (match_token(parserContext, TOKEN_LOGICAL_AND)) {
        ASTNode * rhs = parse_equality_expression(parserContext);
        ASTNode * node = malloc(sizeof(ASTNode));
        node->type = AST_BINARY_EXPR;
        node->binary.op = BINOP_LOGICAL_AND;
        node->binary.lhs = lhs;
        node->binary.rhs = rhs;
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
        root = create_binary_op_node(lhs, binop, rhs);
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
        root = create_binary_op_node(lhs, get_binary_operator_from_tok(op), rhs);
    }
    return root;
}

ASTNode * parse_additive_expression(ParserContext * parserContext) {
    ASTNode * root = parse_term(parserContext);

    while(is_current_token(parserContext, TOKEN_PLUS) || is_current_token(parserContext, TOKEN_MINUS)) {
        ASTNode * lhs = root;
        Token * op = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * rhs = parse_term(parserContext);
        root = create_binary_op_node(lhs, get_binary_operator_from_tok(op), rhs);
    }

    return root;

}


ASTNode * parse_term(ParserContext * parserContext) {
    ASTNode * root = parse_unary_expression(parserContext);

    while(is_current_token(parserContext, TOKEN_STAR) || is_current_token(parserContext,TOKEN_DIV) || is_current_token(parserContext, TOKEN_PERCENT)) {
        ASTNode * lhs = root;
        Token * op = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * rhs = parse_unary_expression(parserContext);
        root = create_binary_op_node(lhs, get_binary_operator_from_tok(op), rhs);
    }

    return root;
}



// ASTNodeType unary_op_token_to_ast_type(TokenType tokenType) {
//     switch(tokenType) {
//         case TOKEN_MINUS: return AST_UNARY_NEGATE; break;
//         case TOKEN_PLUS: return AST_UNARY_PLUS; break;
//         case TOKEN_BANG: return AST_UNARY_NOT; break;
//         case TOKEN_INCREMENT: return AST_UNARY_PRE_INC; break;
//         case TOKEN_DECREMENT: return AST_UNARY_PRE_DEC; break;
//         default:
//             error("Unable to determine prefix operator type");
//             return 0; 
//         break;
    
//     }
// }

ASTNode * parse_unary_expression(ParserContext * parserContext) {
    if (is_current_token(parserContext, TOKEN_PLUS)) {
        expect_token(parserContext, TOKEN_PLUS);
        ASTNode * operand = parse_unary_expression(parserContext);
        return create_unary_node(UNARY_PLUS, operand);
    }
    else if (is_current_token(parserContext, TOKEN_MINUS)) {
        expect_token(parserContext, TOKEN_MINUS);
        ASTNode * operand = parse_unary_expression(parserContext);
        return create_unary_node(UNARY_NEGATE, operand);
    }
    else if (is_current_token(parserContext, TOKEN_BANG)) {
        expect_token(parserContext, TOKEN_BANG);
        ASTNode * operand = parse_unary_expression(parserContext);
        return create_unary_node(UNARY_NOT, operand);
    }
    else if (is_current_token(parserContext, TOKEN_INCREMENT)) {
        expect_token(parserContext, TOKEN_INCREMENT);
        ASTNode * operand = parse_unary_expression(parserContext);
        return create_unary_node(UNARY_PRE_INC, operand);
    }
    else if (is_current_token(parserContext, TOKEN_DECREMENT)) {
        expect_token(parserContext, TOKEN_DECREMENT);
        ASTNode * operand = parse_unary_expression(parserContext);
        return create_unary_node(UNARY_PRE_DEC, operand);
    }
    else {
        return parse_postfix_expression(parserContext);
    }

}

ASTNode*  parse_var_declaration(ParserContext * parserContext) {
    CType * ctype = parse_ctype(parserContext);
    Token * name = expect_token(parserContext, TOKEN_IDENTIFIER);
    ASTNode * expr = NULL;
    if (is_current_token(parserContext, TOKEN_ASSIGN)) {
        advance_parser(parserContext);
        expr = parse_expression(parserContext);
    }
    expect_token(parserContext, TOKEN_SEMICOLON);

    ASTNode * node = create_var_decl_node(name->text, ctype, expr);

    return node;
}

ASTNode * parse_assignment_expression(ParserContext * parserContext) {

    ASTNode * lhs = parse_logical_or(parserContext);

    if (lhs == NULL ) {
        error("lhs must not be null\n");
    }
    else if (!is_lvalue(lhs)) {
        return lhs;
    }

    // Token * name = expect_token(parserContext, TOKEN_IDENTIFIER);
    
    // // if not an assignment fallback to expressio
    // if (!name) {
    //     return parse_expression(parserContext);
    // }

    if (is_current_token(parserContext, TOKEN_ASSIGN)) {
        expect_token(parserContext, TOKEN_ASSIGN);
        ASTNode * rhs = parse_assignment_expression(parserContext);
        
        ASTNode * node =  malloc(sizeof(ASTNode));
        node->type = AST_ASSIGNMENT;
        node->assignment.name = strdup(lhs->var_ref.name);
        node->assignment.expr = rhs;
//        node->assignment.addr.kind = ADDR_UNASSIGNED;
        return node;
    }
    else if (is_current_token(parserContext, TOKEN_PLUS_EQUAL)) {
        expect_token(parserContext, TOKEN_PLUS_EQUAL);

        ASTNode * rhs = parse_expression(parserContext);
        ASTNode * node =  malloc(sizeof(ASTNode));
        node->type = AST_COMPOUND_ADD_ASSIGN;
        node->assignment.name = strdup(lhs->var_ref.name);
        node->assignment.expr = rhs;
//        node->assignment.addr.kind = ADDR_UNASSIGNED;
        return node;
    }
    else if (is_current_token(parserContext, TOKEN_MINUS_EQUAL)) {
            expect_token(parserContext, TOKEN_MINUS_EQUAL); 
            ASTNode * rhs = parse_expression(parserContext);
            ASTNode * node =  malloc(sizeof(ASTNode));
            node->type = AST_COMPOUND_SUB_ASSIGN;
            node->assignment.name = strdup(lhs->var_ref.name);
            node->assignment.expr = rhs;
//            node->assignment.addr.kind = ADDR_UNASSIGNED;
            return node;    
    }
    else {
        return lhs;
    }
    error("Expected assignment operator. actual=%s at line=%d, col=%d\n", 
            get_current_token_type_name(parserContext), get_current_token_line(parserContext), get_current_token_col(parserContext));
    return NULL;  // should never reach because error exits. this is to remove a warning
}


ASTNode * parse_assignment_statement(ParserContext * parserContext) {
    
    ASTNode *expr = parse_assignment_expression(parserContext);
    expect_token(parserContext, TOKEN_SEMICOLON);
    
    ASTNode * node =  malloc(sizeof(ASTNode));
    node->type = AST_EXPRESSION_STMT;
    node->expr_stmt.expr = expr;
    return node;
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

    return primary;
}

// ASTNode * parse_argument_expression_list(ParserContext * parserContext) {
//     ASTNode * node = malloc(sizeof(ASTNode));
//     node->type = AST_ARGUMENT_EXPRESSION_LIST;

//     //TODO

//     return NULL;
// }


ASTNode * parse_primary(ParserContext * parserContext) {

    if (is_current_token(parserContext, TOKEN_INT_LITERAL)) {
        Token * tok = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * node = create_int_literal_node(tok->int_value);
        return node;
    }
    else if (is_current_token(parserContext, TOKEN_LPAREN)) {
        expect_token(parserContext, TOKEN_LPAREN);
        ASTNode * node = parse_expression(parserContext);
        expect_token(parserContext, TOKEN_RPAREN);
        return node;
    }
    else if (is_current_token(parserContext, TOKEN_IDENTIFIER)) {
        Token * tok = peek(parserContext);
        advance_parser(parserContext);
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
            ASTNode * node = create_function_call_node(tok->text, argument_expression_list);
            return node;
        }
        else {
            return create_var_ref_node(tok->text);
        }
    }
    error("Unhandled token error");
    return NULL;

}

