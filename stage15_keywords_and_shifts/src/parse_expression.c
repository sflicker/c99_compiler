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

////////////////////////////////////////////////////////////
////  EXPRESSIONS
////////////////////////////////////////////////////////////
ASTNode * parse_expression(ParserContext * parserContext) {
    return parse_assignment_expression(parserContext);
}

ASTNode_list * parse_argument_expression_list(ParserContext * parserContext) {

    ASTNode_list * arg_list = create_node_list();

    do {
        ASTNode * expression = parse_expression(parserContext);
        ASTNode_list_append(arg_list, expression);
    } while (match_token(parserContext, TOKEN_COMMA));
    return arg_list;
}

ASTNode * parse_assignment_expression(ParserContext * parserContext) {

    ASTNode * lhs = parse_conditional_expression(parserContext);

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

ASTNode * parse_conditional_expression(ParserContext * parserContext) {
    ASTNode * condExpression = parse_logical_or(parserContext);
    if (is_current_token(parserContext, TOKEN_QUESTION_MARK)) {
        expect_token(parserContext, TOKEN_QUESTION_MARK);
        ASTNode * thenExpression = parse_expression(parserContext);
        expect_token(parserContext, TOKEN_COLON);
        ASTNode * elseExpression = parse_conditional_expression(parserContext);
        return create_cond_expr_node(condExpression, thenExpression, elseExpression );
    }

    return condExpression;
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

CType * parse_abstract_declarator(ParserContext * ctx, CType * base) {
    while (match_token(ctx, TOKEN_STAR)) {
        base = make_pointer_type(base);
    }
    return base;
}

ASTNode * parse_cast_expression(ParserContext * parserContext) {
    if (is_current_token(parserContext, TOKEN_LPAREN) && is_next_token_a_ctype(parserContext)) {
        expect_token(parserContext, TOKEN_LPAREN);
        CType * base = parse_type_specifier(parserContext);
        CType * full = parse_abstract_declarator(parserContext, base);
        print_c_type(base, 0);
        expect_token(parserContext, TOKEN_RPAREN);
        ASTNode * expr = parse_cast_expression(parserContext);
        ASTNode * node = create_cast_expr_node(full, expr);
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
    ASTNode * primary = parse_primary_expression(parserContext);

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
    ASTNode * node = parse_primary_expression(parserContext);
    if (node->type == AST_INT_LITERAL) {
        return node;
    }
    error("expected constant expression\n");
    return NULL;
}

ASTNode * parse_primary_expression(ParserContext * parserContext) {

    // handle integer literals
    if (is_current_token(parserContext, TOKEN_INT_LITERAL)) {
        Token * tok = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * node = create_int_literal_node(tok->int_value);
        return node;
    }
    if (is_current_token(parserContext, TOKEN_FLOAT_LITERAL)) {
        Token * tok = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * node = create_float_literal_node(tok->float_value);
        return node;
    }
    if (is_current_token(parserContext, TOKEN_DOUBLE_LITERAL)) {
        Token * tok = peek(parserContext);
        advance_parser(parserContext);
        ASTNode * node = create_double_literal_node(tok->double_value);
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
    if (is_current_token(parserContext, TOKEN_STRING_LITERAL)) {
        Token * tok = peek(parserContext);
        advance_parser(parserContext);
        return create_string_literal_node(tok->text);
    }
    error("Unhandled token error, %s", get_current_token_type_name(parserContext));
    return NULL;

}
