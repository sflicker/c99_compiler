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
                    | <assignment_statement>
                    | <return_statement>
                    | <if_statement>
                    | <while_statement>
                    | <for_statement>
                    | <block>
                    | <expression_stmt>

   <declaration> ::= "int" <identifier> [ "=" <expression> ] ";"

   <assignment_statement> ::= <assignment_expression> ";";

   <assignment_expression> ::=   <identifier> "=" <expression>
                               | <identifier> '+=' <expression> 
                               | <identifier> '-=' <expression>

   <return_statement>     ::= "return" <expression> ";"

   <if_statement> ::= "if" "(" <expression> ")" <statement> [ "else" <statement> ]

   <while_statement> ::= "while" "(" <expression> ")" <statement> 

   <for_statement> ::= "for" "(" [init_expr] ";" [cond_expr] ";" [update_expr] ")" statement

   <expression_stmt> ::= <expression> ";"

   <expression> ::= <equality_expr>

   <equality_expr> ::= <relational_expr> [ ("==" | "!=") <relational_expr> ]*

   <relational_expr> ::= <additive_expr> [ ( "<" | "<=" | ">" | ">=") <additive_expr> ]*

   <additive_expr> ::= <term> [ ("+" | "-") <term> ]*

   <term>       ::= <unary_expr> [ ("*" | "/") <unary_expr> ]*
   
   <unary_expr> ::= [ "+" | "-" | "!" | "++"" | "--"" ] <unary_expr> | <primary>

   <postfix_expr> ::= <primary> [ "++" | "--" ]?

   <primary>     ::= <int_literal> 
                    | <identifier>
                    | "(" <expression> ")"

   <identifier>    ::= [a-zA-Z_][a-zA-Z0-9_]*

    <int_literal> ::= [0-9]+
*/

char currentTokenInfo[128];
char nextTokenInfo[128];

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
ASTNode * parse_postfix_expression(ParserContext * parserContext);
ASTNode * parse_expression(ParserContext * parserContext);
ASTNode * parse_primary(ParserContext * parserContext);
ASTNode*  parse_var_declaration(ParserContext * parserContext);
ASTNode* parse_assignment_statement(ParserContext * parserContext);
ASTNode* parse_while_statement(ParserContext * parserContext);
ASTNode * parse_for_statement(ParserContext * parserContext);
ASTNode * parse_assignment_expression(ParserContext * parserContext);

void update_current_token_info(ParserContext* parserContext);

void initialize_parser(ParserContext * parserContext, TokenList * tokenList) {
    parserContext->list = tokenList;
    parserContext->pos = 0;
    update_current_token_info(parserContext);
}

void update_current_token_info(ParserContext* ctx) {
    Token * currentToken = (ctx->pos < ctx->list->count) ? &ctx->list->data[ctx->pos] : NULL;
    if (currentToken != NULL) {
        snprintf(currentTokenInfo, sizeof(currentTokenInfo), "POS: %d, TOKEN: %s, TEXT: %s", 
            ctx->pos,
            token_type_name(currentToken->type), currentToken->text ? currentToken->text : "(null)");
    }
    else {
        snprintf(currentTokenInfo, sizeof(currentTokenInfo), "(NULL)\n");
    }

    Token * nextToken = (ctx->pos+1 < ctx->list->count) ? &ctx->list->data[ctx->pos+1] : NULL;
    if (nextToken != NULL) {
        snprintf(nextTokenInfo, sizeof(nextTokenInfo), "POS: %d, TOKEN: %s, TEXT: %s", 
            ctx->pos+1,
            token_type_name(nextToken->type), nextToken->text ? nextToken->text : "(null)");
    }
    else {
        snprintf(nextTokenInfo, sizeof(nextTokenInfo), "(NULL)\n");
    }
}

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
    update_current_token_info(parserContext);
    return token;
}

bool match_token(ParserContext * parserContext, TokenType type) {
    if (parserContext->list->data[parserContext->pos].type == type) {
        advance(parserContext);
        return true;
    }
    return false;
}

Token* expect_token(ParserContext * parserContext, TokenType expected) {
    Token * token = peek(parserContext);
    if (token->type == expected) {
        return advance(parserContext);
    }

    printf("unexpected token at POS: %d, expected: %s, actual: %s\n", parserContext->pos, token_type_name(expected), token_type_name(token->type));


//    fprintf(stderr, "Parse error: expected token of type %s, but got %s (text: '%.*s')\n",
//        token_type_name(expected), token->type, token->length, token->text);

    exit(1);    
}

ASTNodeType binary_op_token_to_ast_type(TokenType tok) {
    switch (tok) {
        case TOKEN_PLUS: return AST_ADD;
        case TOKEN_MINUS: return AST_SUB;
        case TOKEN_STAR: return AST_MUL;
        case TOKEN_DIV: return AST_DIV;
        case TOKEN_EQ: return AST_EQUAL;
        case TOKEN_NEQ: return AST_NOT_EQUAL;
        case TOKEN_LT: return AST_LESS_THAN;
        case TOKEN_LE: return AST_LESS_EQUAL;
        case TOKEN_GT: return AST_GREATER_THAN;
        case TOKEN_GE: return AST_GREATER_EQUAL;
        default:
            fprintf(stderr, "Unknown binary operator in expression");
            exit(1);
    }
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

bool is_next_token_assignment(ParserContext * parserContext) {
    return is_next_token(parserContext, TOKEN_ASSIGN) || 
        is_next_token(parserContext, TOKEN_PLUS_EQUAL) || 
        is_next_token(parserContext, TOKEN_MINUS_EQUAL);
}

ASTNode * parse_statement(ParserContext* parserContext) {
    if (is_current_token(parserContext, TOKEN_INT)) {
        return parse_var_declaration(parserContext);
    }
    if (is_current_token(parserContext, TOKEN_IDENTIFIER) && is_next_token_assignment(parserContext)){
        return parse_assignment_statement(parserContext);
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
        if (is_current_token(parserContext, TOKEN_IDENTIFIER) && is_next_token(parserContext, TOKEN_ASSIGN)) {
            update_expr = parse_assignment_expression(parserContext);
        }
        else {
            update_expr = parse_expression(parserContext);
        }
    }
    expect_token(parserContext, TOKEN_RPAREN);

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
    ASTNodeType nodeType = binary_op_token_to_ast_type(op);
    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = nodeType;
    node->binary.lhs = lhs;
//    node->binary_op.op = op;
    node->binary.rhs = rhs;
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

ASTNode * create_unary_node(ASTNodeType op, ASTNode * operand) {
    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = op;
    node->unary.operand = operand;
    return node;
}

ASTNodeType unary_op_token_to_ast_type(TokenType tokenType) {
    switch(tokenType) {
        case TOKEN_MINUS: return AST_UNARY_NEGATE; break;
        case TOKEN_PLUS: return AST_UNARY_PLUS; break;
        case TOKEN_BANG: return AST_UNARY_NOT; break;
        case TOKEN_INCREMENT: return AST_UNARY_PRE_INC; break;
        case TOKEN_DECREMENT: return AST_UNARY_PRE_DEC; break;
        default:
            error("Unable to determine prefix operator type");
            return 0; 
        break;
    
    }
}

ASTNode * parse_unary_expression(ParserContext * parserContext) {
    if (is_current_token(parserContext, TOKEN_PLUS) 
     || is_current_token(parserContext, TOKEN_MINUS) 
     || is_current_token(parserContext, TOKEN_BANG)
     || is_current_token(parserContext, TOKEN_INCREMENT) 
     || is_current_token(parserContext, TOKEN_DECREMENT)) {
                Token * currentToken = advance(parserContext);

                ASTNode * operand = parse_unary_expression(parserContext);
                ASTNode * node = create_unary_node(unary_op_token_to_ast_type(currentToken->type), operand);

                return node;
            }        
    else {
        return parse_postfix_expression(parserContext);
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

ASTNode * parse_assignment_expression(ParserContext * parserContext) {
    Token * name = expect_token(parserContext, TOKEN_IDENTIFIER);
    
    // if not an assignment fallback to expressio
    if (!name) {
        return parse_expression(parserContext);
    }

    if (is_current_token(parserContext, TOKEN_ASSIGN)) {
        expect_token(parserContext, TOKEN_ASSIGN);
        ASTNode * expr = parse_expression(parserContext);
        
        ASTNode * node =  malloc(sizeof(ASTNode));
        node->type = AST_ASSIGNMENT;
        node->assignment.name = my_strdup(name->text);
        node->assignment.expr = expr;
        return node;
    }
    else if (is_current_token(parserContext, TOKEN_PLUS_EQUAL)) {
        expect_token(parserContext, TOKEN_PLUS_EQUAL);

        ASTNode * rhs = parse_expression(parserContext);
        ASTNode * node =  malloc(sizeof(ASTNode));
        node->type = AST_COMPOUND_ADD_ASSIGN;
        node->assignment.name = my_strdup(name->text);
        node->assignment.expr = rhs;
        return node;
    }
    else if (is_current_token(parserContext, TOKEN_MINUS_EQUAL)) {
            expect_token(parserContext, TOKEN_MINUS_EQUAL); 
            ASTNode * rhs = parse_expression(parserContext);
            ASTNode * node =  malloc(sizeof(ASTNode));
            node->type = AST_COMPOUND_SUB_ASSIGN;
            node->assignment.name = my_strdup(name->text);
            node->assignment.expr = rhs;
            return node;    
    }
    error("Expected assignment operator");
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

        return create_unary_node(AST_UNARY_POST_INC, primary);
    }
    if (is_current_token(parserContext, TOKEN_DECREMENT)) {
        expect_token(parserContext, TOKEN_DECREMENT);

        return create_unary_node(AST_UNARY_POST_DEC, primary);
    }

    return primary;
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
        case AST_ADD:
            printf("Binary: %s\n", "ADD");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;
        case AST_SUB:
            printf("Binary: %s\n", "SUB");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;
        case AST_MUL:
            printf("Binary: %s\n", "MULTIPLY");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;
        case AST_DIV:
            printf("Binary: %s\n", "DIVIDE");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;
        case AST_EQUAL:
            printf("Binary: %s\n", "EQUAL");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;
        case AST_NOT_EQUAL:
            printf("Binary: %s\n", "NOTEQUAL");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;
        case AST_LESS_THAN:
            printf("Binary: %s\n", "LESSTHAN");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;
        case AST_LESS_EQUAL:
            printf("Binary: %s\n", "LESSEQUAL");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;
        case AST_GREATER_THAN:
            printf("Binary: %s\n", "GREATERTHAN");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;
        case AST_GREATER_EQUAL:
            printf("Binary: %s\n", "GREATEREQUAL");
            print_ast(node->binary.lhs, indent+1);
            print_ast(node->binary.rhs, indent+1);
            break;

        case AST_UNARY_NEGATE:
            printf("Unary: %s\n", "NEGATE");
            print_ast(node->unary.operand, indent+1);
            break;
        case AST_UNARY_NOT:
            printf("Unary: %s\n", "NOT");
            print_ast(node->unary.operand, indent+1);
            break;
        case AST_UNARY_PLUS:
            printf("Unary: %s\n", "PLUS");
            print_ast(node->unary.operand, indent+1);
            break;
        case AST_UNARY_PRE_INC:
            printf("Unary: %s\n", "PREFIX_INC");
            print_ast(node->unary.operand, indent+1);
            break;
        case AST_UNARY_PRE_DEC:
            printf("Unary: %s\n", "PREFIX_DEC");
            print_ast(node->unary.operand, indent+1);
            break;
        case AST_UNARY_POST_INC:
            printf("Unary: %s\n", "POSTFIX_INC");
            print_ast(node->unary.operand, indent+1);
            break;
        case AST_UNARY_POST_DEC:
            printf("Unary: %s\n", "POSTFIX_DEC");
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
        case AST_COMPOUND_ADD_ASSIGN:
            printf("AddAssign: %s\n", node->assignment.name);
            print_ast(node->assignment.expr, indent+1);
            break;
        case AST_COMPOUND_SUB_ASSIGN:
            printf("SubAssign: %s\n", node->assignment.name);
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
