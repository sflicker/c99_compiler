#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

/* 
   Simple C compiler example 

   Simplifed grammar

   <program>       ::= <function_decl>

   <function_decl> ::= "int" <identifier> "(" ")" "{" <statement> "}"

   <statement>     ::= "return" <expression> ";"

   <expression>    ::= <integer_literal>

   <identifier>    ::= [a-zA-Z_][a-zA-Z0-9_]*
    <integer_literal> ::= [0-9]+
*/

typedef enum {
    TOKEN_EOF,
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    const char * text;
    int length;
    int int_value;
} Token;

typedef struct {
    Token * data;
    int capacity;
    int count;
} TokenList;

typedef enum {
    AST_RETURN_STMT,
    AST_INT_LITERAL,
    AST_FUNCTION,
    AST_PROGRAM
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int int_value;

        struct {
            struct ASTNode * expr;
        } return_stmt;

        struct {
            const char* name;
            struct ASTNode* body;
        } function;

        struct {
            struct ASTNode * function;
        } program;
    };
} ASTNode;

typedef struct {
    TokenList* list;
    int pos;
} ParserContext;

ASTNode* parse_program(ParserContext * parserContext);
ASTNode* parse_function(ParserContext * parserContext);
ASTNode * parse_statement(ParserContext* parserContext);
ASTNode* parse_return_statement(ParserContext * parserContext);
ASTNode* parse_expression(ParserContext* parserContext);

Token* peek(ParserContext * parserContext);
Token* advance(ParserContext * parserContext);
bool match_token(ParserContext* p, TokenType type);

char * my_strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char * copy = malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

Token* expect_token(ParserContext * parserContext, TokenType expected);

void init_token_list(TokenList * list) {
    list->capacity = 16;
    list->count = 0;
    list->data = malloc(sizeof(Token) * list->capacity);
}

void cleanup_token_list(TokenList * tokenList) {
    for (int i=0;i<tokenList->count;i++) {
        free((void*)tokenList->data[i].text);
    }
}

void add_token(TokenList * list, Token token) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->data = realloc(list->data, sizeof(Token) * list->capacity);
    }
    list->data[list->count++] = token;
}

const char *keywords[] = {
    "int", "return"
};

const int num_keywords = sizeof(keywords)/sizeof(keywords[0]);

bool is_keyword(const char * word) {
    for (int i=0;i<num_keywords; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}


const char * sampleProgram = "int main() { \n"
                             "    return 42; \n"
                             "}";


bool is_punctuator(char c) {
    return c == '{' || c == '}' || c == '(' || c == ')' || c == ';';
}

TokenType punctuator_token(char punctuator) {
    switch (punctuator) {
        case '{': return TOKEN_LBRACE;
        case '}': return TOKEN_RBRACE;
        case '(': return TOKEN_LPAREN;
        case ')': return TOKEN_RPAREN;
        case ';': return TOKEN_SEMICOLON;
        default : return TOKEN_UNKNOWN;
    }
}

TokenType get_keyword_token(const char* keyword) {
    if (strcmp(keyword, "int") == 0) {
        return TOKEN_INT;
    }
    else if (strcmp(keyword, "return") == 0) {
        return TOKEN_RETURN;
    }
    else {
        return TOKEN_UNKNOWN;
    }
}

const char * token_type_name(TokenType type) {
    switch(type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_INT: return "INT";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INT_LITERAL: return "LITERAL_INT";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        default: return "UNKNOWN";
    }
}

void formatted_output(const char * label, const char * text, TokenType tokenType) {
    char left[64];
    char right[64];
    snprintf(left, sizeof(left), "%s: %s", label, text);
    snprintf(right, sizeof(right), "TOKEN_TYPE: %s", token_type_name(tokenType));
    printf("%-25s %-25s\n", left, right);
}

void add_keyword_token(TokenList *tokenList, const char * keywordText) {
    Token token;
    token.type = get_keyword_token(keywordText);
    int buffer_len = strlen(keywordText);
    char * buffer_copy = malloc(buffer_len + 1);
    memcpy(buffer_copy, keywordText, buffer_len);
    buffer_copy[buffer_len] = '\0';
    token.text = buffer_copy;
    token.length = buffer_len;
    token.int_value = 0;
    add_token(tokenList, token);
}

void add_identifier_token(TokenList * tokenList, const char * id) {
    Token token;
    token.type = TOKEN_IDENTIFIER;
    int idLen = strlen(id);
    char * idCopy = malloc(idLen + 1);
    memcpy(idCopy, id, idLen);
    token.text = idCopy;
    token.length = idLen;
    token.int_value = 0;
    add_token(tokenList, token);
}

void add_int_token(TokenList * tokenList, char * numberText) {
    Token token;
    token.type = TOKEN_INT_LITERAL;
    int numberTextLen = strlen(numberText);
    char * numberTextCopy = malloc(numberTextLen + 1);
    memcpy(numberTextCopy, numberText, numberTextLen);
    token.text = numberTextCopy;
    token.length = numberTextLen;
    token.int_value = atoi(numberText);
    add_token(tokenList, token);
}


void add_punctuator_token(TokenList * tokenList, const char * punctuatorText) {
    Token token;
    token.type = punctuator_token(*punctuatorText);
    int punctuatorLen = 1;
    char * punctuatorCopy = malloc(punctuatorLen + 1);
    memcpy(punctuatorCopy, punctuatorText, punctuatorLen);
    token.text = punctuatorCopy;
    token.length = punctuatorLen;
    token.int_value = 0;
    add_token(tokenList, token);
}

void tokenize(const char* code, TokenList * tokenList) {
    const char* p = code;
    
    init_token_list(tokenList);

    while(*p) {
        if (isspace(*p)) {
            p++;
        }
        else if (isalpha(*p) || *p == '_') {
            char buffer[128];
            int i=0;
            while(isalnum(*p) || *p == '_') {
                buffer[i++] = *p++;
            }
            buffer[i] = '\0';

            if (is_keyword(buffer)) {
                add_keyword_token(tokenList, buffer);

//                formatted_output("KEYWORD:", buffer, tokenType);
            }
            else {
                add_identifier_token(tokenList, buffer);

 //               formatted_output("IDENTIFIER:", buffer, tokenType);
            }
        }
        else if (isdigit(*p)) {
            char buffer[128];
            int i=0;
            while(isdigit(*p)) {
                buffer[i++] = *p++;
            }
            buffer[i++] = '\0';
            add_int_token(tokenList, buffer);
//            formatted_output("INTEGER:" , buffer, tokenType);
        }
        else if (is_punctuator(*p)) {
            char buffer[2] = { *p, '\0'};
            add_punctuator_token(tokenList, buffer);

//            formatted_output("PUNCTUATOR:", buffer, tokenType);
            p++;
        }
    }


}

Token * peek(ParserContext * parserContext) {
    return &parserContext->list->data[parserContext->pos];
}

bool is_next_token(ParserContext * parserContext, TokenType type) {
    return parserContext->list->data[parserContext->pos].type == type;
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
    expect_token(parserContext, TOKEN_LBRACE);
    
    ASTNode* return_stmt = parse_statement(parserContext);

    expect_token(parserContext, TOKEN_RBRACE);

    ASTNode * func = malloc(sizeof(ASTNode));
    func->type = AST_FUNCTION;
    func->function.name = my_strdup(name->text);
    func->function.body = return_stmt;
    return func;
}

ASTNode * parse_statement(ParserContext* parserContext) {
    if (is_next_token(parserContext, TOKEN_RETURN)) {
        return parse_return_statement(parserContext);
    }
    printf("unsupported statement\n");
    exit(1);
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
    Token * tok = expect_token(parserContext, TOKEN_INT_LITERAL);

    ASTNode * node = malloc(sizeof(ASTNode));
    node->type = AST_INT_LITERAL;
    node->int_value = tok->int_value;
    return node;
}


int main() {
    TokenList tokenList;

    tokenize(sampleProgram, &tokenList);

    // output list
    for (int i=0;i<tokenList.count;i++) {
        Token token = tokenList.data[i];
        formatted_output("TOKEN:", token.text, token.type);
    }

    ParserContext parserContext;
    parserContext.list = &tokenList;
    parserContext.pos = 0;

    ASTNode * program = parse_program(&parserContext);

    cleanup_token_list(&tokenList);
    
}                             