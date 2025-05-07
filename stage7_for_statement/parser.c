#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "token.h"
#include "ast.h"
#include "parser.h"

/* 
   Simple C compiler example 

   Simplifed grammar

   <program>       ::= <function>

   <function> ::= "int" <identifier> "(" ")" <block>

   <block> ::= "{" { <statement> } "}"

   <statement> ::=  <declaration>
                    | <assignment>
                    | <return_statement>
                    | <if_statement>
                    | <while_statement>
                    | <for_statement>
                    | <block>
                    | <expression_stmt>

   <declaration> ::= "int" <identifier> [ "=" <expression> ] ";"

   <assignment> ::= <identifier> "=" <expression> ";";

   <return_statement>     ::= "return" <expression> ";"

   <if_statement> ::= "if" "(" <expression> ")" <statement> [ "else" <statement> ]

   <while_statement> ::= "while" "(" <expression> ")" <statement> 

   <for_statement> ::= "for" "(" [init_expr] ";" [cond_expr] ";" [update_expr] ")" statement

   <expression_stmt> ::= <expression> ";";

   <expression> ::= <equality_expr>

   <equality_expr> ::= <relational_expr> [ ("==" | "!=") <relational_expr> ]*

   <relational_expr> ::= <additive_expr> [ ( "<" | "<=" | ">" | ">=") <additive_expr> ]*

   <additive_expr> ::= <term> [ ("+" | "-") <term> ]*

   <term>       ::= <unary_expr> [ ("*" | "/") <unary_expr> ]*
   
   <unary_expr> ::= [ "+" | "-" | "!" ] <unary_expr> | <primary>

   <primary>     ::= <int_literal> 
                    | <identifier>
                    | "(" <expression> ")"

   <identifier>    ::= [a-zA-Z_][a-zA-Z0-9_]*

    <int_literal> ::= [0-9]+
*/

Token* expect_token(ParserContext * parserContext, TokenType expected);

Token* peek(ParserContext * parserContext);
Token* advance(ParserContext * parserContext);
bool match_token(ParserContext* p, TokenType type);

ASTNode* parse_function(ParserContext * parserContext);
ASTNode * parse_statement(ParserContext* parserContext);
ASTNode* parse_return_statement(ParserContext * parserContext);
ASTNode * parse_block(ParserContext* parserContext);
ASTNode * parse_if_statement(ParserContext * parserContext);
ASTNode * parse_expression_statement(ParserContext * parserContext);
ASTNode* parse_expression(ParserContext* parserContext);
ASTNode * parse_equality_expression(ParserContext * parserContext);
ASTNode * parse_relational_expression(ParserContext * parserContext);
ASTNode * parse_additive_expression(ParserContext * parserContext);
ASTNode * parse_term(ParserContext * parserContext);
ASTNode * parse_unary_expression(ParserContext * parserContext);
ASTNode * parse_expression(ParserContext * parserContext);
ASTNode * parse_primary(ParserContext * parserContext);
ASTNode*  parse_var_declaration(ParserContext * parserContext);
ASTNode* parse_assignment(ParserContext * parserContext);
ASTNode* parse_while_statement(ParserContext * parserContext);
ASTNode * parse_for_statement(ParserContext * parserContext);

Token * peek(ParserContext * parserContext) {
    return &parserContext->list->data[parserContext->pos];
}

bool is_current_token(ParserContext * parserContext, TokenType type) {
    return parserContext->list->data[parserContext->pos].type == type;
}

//TODO need a check so this doesn't cause an out of bounds
bool is_next_token(ParserContext * parserContext, TokenType type) {
    return parserContext->list->data[parserContext->pos+1].type == type;
}

Token * advance(ParserContext * parserContext) {
    Token * token = peek(parserContext);
    parserContext->pos++;
    return token;
}

bool match_token(ParserContext * parserContext, TokenType type) {
    if (parserContext->list->data[parserContext->pos].type == type) {
        parserContext->pos++;
        return true;
    }
    return false;
}

Token* expect_token(ParserContext * parserContext, TokenType expected) {
    Token * token = peek(parserContext);
    if (token->type == expected) {
        return advance(parserContext);
    }

    printf("unexpected token\n");


//    fprintf(stderr, "Parse error: expected token of type %s, but got %s (text: '%.*s')\n",
//        token_type_name(expected), token->type, token->length, token->text);

    exit(1);    
}


ASTNode * parse_program(ParserContext * parserContext) {
    ASTNode * function = parse_function(parserContext);

    ASTNode * programNode = malloc(sizeof(ASTNode));
    programNode->type = AST_PROGRAM;
    programNode->program.function = function;
    return programNode;
}

ASTNode * parse_function(ParserContext* parserContext) {
    expect_token(parserContext, TOKEN_INT);
    Token* name = expect_token(parserContext, TOKEN_IDENTIFIER);
    expect_token(parserContext, TOKEN_LPAREN);
    expect_token(parserContext, TOKEN_RPAREN);
//    expect_token(parserContext, TOKEN_LBRACE);
    
    ASTNode* function_block = parse_block(parserContext);

//    expect_token(parserContext, TOKEN_RBRACE);

    ASTNode * func = malloc(sizeof(ASTNode));
    func->type = AST_FUNCTION;
    func->function.name = my_strdup(name->text);
    func->function.body = function_block;
    return func;
}

ASTNode * parse_block(ParserContext* parserContext) {
    expect_token(parserContext, TOKEN_LBRACE);

    ASTNode * blockNode = malloc(sizeof(ASTNode));
    blockNode->type = AST_BLOCK;
    blockNode->block.count = 0;
    blockNode->block.capacity = 4;
    blockNode->block.statements = malloc(sizeof(ASTNode*) * blockNode->block.capacity);

    while(!is_current_token(parserContext, TOKEN_RBRACE)) {
        ASTNode * statement = parse_statement(parserContext);
        if (blockNode->block.count >= blockNode->block.capacity) {
            blockNode->block.capacity *= 2;
            blockNode->block.statements = realloc(
                blockNode->block.statements, sizeof(ASTNode*) * blockNode->block.capacity);
        }
        blockNode->block.statements[blockNode->block.count++] = statement;
    }
        
    expect_token(parserContext, TOKEN_RBRACE);

    return blockNode;
}

ASTNode * parse_statement(ParserContext* parserContext) {
    if (is_current_token(parserContext, TOKEN_INT)) {
        return parse_var_declaration(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_IDENTIFIER) && is_next_token(parserContext, TOKEN_ASSIGN)) {
        return parse_assignment(parserContext);
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
    if (is_current_token(parserContext, TOKEN_FOR)) {
        return parse_for_statement(parserContext);
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
        advance(parserContext);
        else_statement = parse_statement(parserContext);
    }

    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_IF_STMT;
    node->if_stmt.cond = condExpression;
    node->if_stmt.then_statement = then_statement;
    node->if_stmt.else_statement = else_statement;
    return node;

}

ASTNode * parse_while_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_WHILE);
    expect_token(parserContext, TOKEN_LPAREN);
    ASTNode * condExpression = parse_expression(parserContext);
    expect_token(parserContext, TOKEN_RPAREN);
    ASTNode * body_statement = parse_statement(parserContext);

    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE_STMT;
    node->while_stmt.cond = condExpression;
    node->while_stmt.body = body_statement;
    return node;

}

ASTNode * parse_for_statement(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_FOR);
    expect_token(parserContext, TOKEN_LPAREN);

    ASTNode * init_expr = NULL;
    ASTNode * cond_expr = NULL;
    ASTNode * update_expr = NULL;

    if (!is_current_token(parserContext, TOKEN_SEMICOLON)) {
        init_expr = parse_expression_statement(parserContext);
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

    if (!is_current_token(parserContext, TOKEN_SEMICOLON)) {
        init_expr = parse_expression(parserContext);
    }
    expect_token(parserContext, TOKEN_SEMICOLON);

    if (!is_current_token(parserContext, TOKEN_RPAREN)) {
        update_expr = parse_expression(parserContext);
    }
    expect_token(parserContext, TOKEN_SEMICOLON);

    ASTNode * body = parse_statement(parserContext);

    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_FOR_STMT;
    node->for_stmt.init_expr = init_expr;
    node->for_stmt.cond_expr = cond_expr;
    node->for_stmt.update_expr = update_expr;
    node->for_stmt.body = body;

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

ASTNode * create_binary_op(ASTNode * lhs, TokenType op, ASTNode *rhs) {
    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_BINARY_OP;
    node->binary_op.lhs = lhs;
    node->binary_op.op = op;
    node->binary_op.rhs = rhs;
    return node;    
}

ASTNode * parse_expression(ParserContext * parserContext) {
    return parse_equality_expression(parserContext);
}

ASTNode * parse_equality_expression(ParserContext * parserContext) {
    ASTNode * root = parse_relational_expression(parserContext);

    while(is_current_token(parserContext, TOKEN_EQ) || is_current_token(parserContext, TOKEN_NEQ)) {
        ASTNode * lhs = root;
        Token * op = advance(parserContext);
        ASTNode * rhs = parse_relational_expression(parserContext);
        root = create_binary_op(lhs, op->type, rhs);
    }

    return root;
}

ASTNode * parse_relational_expression(ParserContext * parserContext) {
    ASTNode * root = parse_additive_expression(parserContext);
    
    while(is_current_token(parserContext, TOKEN_GT) || is_current_token(parserContext, TOKEN_GE) 
            || is_current_token(parserContext, TOKEN_LT) || is_current_token(parserContext, TOKEN_LE)) {
        ASTNode * lhs = root;
        Token * op = advance(parserContext);
        ASTNode * rhs = parse_additive_expression(parserContext);
        root = create_binary_op(lhs, op->type, rhs);
    }
    return root;
}

ASTNode * parse_additive_expression(ParserContext * parserContext) {
    ASTNode * root = parse_term(parserContext);

    while(is_current_token(parserContext, TOKEN_PLUS) || is_current_token(parserContext, TOKEN_MINUS)) {
        ASTNode * lhs = root;
        Token * op = advance(parserContext);
        ASTNode * rhs = parse_term(parserContext);
        root = create_binary_op(lhs, op->type, rhs);
    }

    return root;

}


ASTNode * parse_term(ParserContext * parserContext) {
    ASTNode * root = parse_unary_expression(parserContext);

    while(is_current_token(parserContext, TOKEN_STAR) || is_current_token(parserContext,TOKEN_DIV)) {
        ASTNode * lhs = root;
        Token * op = advance(parserContext);
        ASTNode * rhs = parse_unary_expression(parserContext);
        root = create_binary_op(lhs, op->type, rhs);
    }

    return root;
}

ASTNode * parse_unary_expression(ParserContext * parserContext) {
    if (is_current_token(parserContext, TOKEN_PLUS) || is_current_token(parserContext, TOKEN_MINUS) 
            || is_current_token(parserContext, TOKEN_BANG)) {
                Token * currentToken = advance(parserContext);
                TokenType op = currentToken->type;

                ASTNode * expr = parse_unary_expression(parserContext);
                
                ASTNode * node = malloc(sizeof(ASTNode));
                node->type = AST_UNARY_OP;
                node->unary_op.op = op;
                node->unary_op.expr = expr;
                return node;
            }        
    else {
        return parse_primary(parserContext);
    }

}

ASTNode*  parse_var_declaration(ParserContext * parserContext) {
    expect_token(parserContext, TOKEN_INT);
    Token * name = expect_token(parserContext, TOKEN_IDENTIFIER);
    ASTNode * expr = NULL;
    if (is_current_token(parserContext, TOKEN_ASSIGN)) {
        advance(parserContext);
        expr = parse_expression(parserContext);
    }
    expect_token(parserContext, TOKEN_SEMICOLON);

    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_VAR_DECL;
    node->declaration.name = my_strdup(name->text);
    node->declaration.init_expr = expr;
    return node;
}

ASTNode * parse_assignment(ParserContext * parserContext) {
    Token * name = expect_token(parserContext, TOKEN_IDENTIFIER);
    expect_token(parserContext, TOKEN_ASSIGN);
    ASTNode * expr = parse_expression(parserContext);
    expect_token(parserContext, TOKEN_SEMICOLON);
    
    ASTNode * node =  malloc(sizeof(ASTNode));
    node->type = AST_ASSIGNMENT;
    node->assignment.name = my_strdup(name->text);
    node->assignment.expr = expr;
    return node;
}

ASTNode * parse_primary(ParserContext * parserContext) {

    if (is_current_token(parserContext, TOKEN_INT_LITERAL)) {
        Token * tok = advance(parserContext);
        ASTNode * node = malloc(sizeof(ASTNode));
        node->type = AST_INT_LITERAL;
        node->int_value = tok->int_value;
        return node;
    }
    else if (is_current_token(parserContext, TOKEN_LPAREN)) {
        expect_token(parserContext, TOKEN_LPAREN);
        ASTNode * node = parse_expression(parserContext);
        expect_token(parserContext, TOKEN_RPAREN);
        return node;
    }
    else if (is_current_token(parserContext, TOKEN_IDENTIFIER)) {
        Token * tok = advance(parserContext);
        ASTNode * node = malloc(sizeof(ASTNode));
        node->type = AST_VAR_EXPR;
        node->var_expression.name = my_strdup(tok->text);
        return node;
    }

    return NULL;

}

void print_ast(ASTNode * node, int indent) {
    if(!node) return;

    for (int i=0;i<indent;i++) printf("  ");

    switch(node->type) {
        case AST_PROGRAM:
            printf("ProgramDecl:\n");
            print_ast(node->program.function, 1);
            break;
        case AST_FUNCTION:
            printf("FunctionDecl: %s\n", node->function.name);
            print_ast(node->function.body, indent+1);
            break;
        case AST_RETURN_STMT:
            printf("ReturnStmt:\n");
            print_ast(node->return_stmt.expr, indent+1);
            break;
        case AST_INT_LITERAL:
            printf("IntLiteral: %d\n", node->int_value);
            break;
        case AST_BINARY_OP:
            printf("BinaryOp: %s\n", token_type_name(node->binary_op.op));
            print_ast(node->binary_op.lhs, indent+1);
            print_ast(node->binary_op.rhs, indent+1);
            break;
        case AST_UNARY_OP:
            printf("UnaryOp: %s\n", token_type_name(node->unary_op.op));
            print_ast(node->unary_op.expr, indent+1);
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
        case AST_BLOCK:
            printf("Block\n");
            for (int i=0;i<node->block.count;i++) {
                print_ast(node->block.statements[i], indent+1);
            }
            break;
        case AST_EXPRESSION_STMT:
            printf("ExpressionStatement\n");
            print_ast(node->expr_stmt.expr, indent+1);
            break;
        case AST_VAR_DECL:
            printf("VariableDeclaration: %s\n", node->declaration.name);
            if (node->declaration.init_expr) {
                print_ast(node->declaration.init_expr, indent+1);
            }
            break;
        case AST_ASSIGNMENT:
            printf("Assignment: %s\n", node->assignment.name);
            print_ast(node->assignment.expr, indent+1);
            break;
        case AST_VAR_EXPR:
            printf("VariableExpression: %s\n", node->var_expression.name);
            break;
        default:
            printf("Unknown AST Node Type: %d\n", node->type);
            break;
    }
}
