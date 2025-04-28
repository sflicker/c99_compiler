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

   <expression> ::= <equality_expr>

   <equality_expr> ::= <relational_expr> ( ("==" | "!=") <relational_expr> )*

   <relational_expr> ::= <additive_expr> ( ( "<" | "<=" | ">" | ">=") <additive_expr> )*

   <additive_expr> ::= <term> { ("+" | "-") <term> }*

   <term>       ::= <factor> { ("*" | "/") <factor> }*
   
   <factor>     ::= <int_literal> | "(" <expression> ")"

   <identifier>    ::= [a-zA-Z_][a-zA-Z0-9_]*

    <int_literal> ::= [0-9]+
*/


// const char * sampleProgram = "int main() { \n"
// "    return 7 * (2 + 4); \n"
// "}";

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
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_DIV,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LE,
    TOKEN_LT,
    TOKEN_GE,
    TOKEN_GT,
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
    AST_PROGRAM,
    AST_BINARY_OP
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

        struct {
            struct ASTNode * lhs;
            TokenType op;
            struct ASTNode * rhs;
        } binary_op;

    };
} ASTNode;

typedef struct {
    TokenList* list;
    int pos;
} ParserContext;

const char * token_type_name(TokenType type);

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

void add_token_by_type(TokenList * list, TokenType tokenType) {
    const char * tokenText = token_type_name(tokenType);
    int tokenLen = strlen(tokenText);
    char * tokenCopy = malloc(tokenLen + 1);
    memcpy(tokenCopy, tokenText, tokenLen);
    tokenCopy[tokenLen] = '\0';

    Token token;
    token.type = tokenType;
    token.text = tokenCopy;
    token.int_value = 0;
    token.length = tokenLen;
    add_token(list, token);
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



bool is_punctuator(char c) {
    return c == '{' || c == '}' || c == '(' || c == ')' || c == ';';
}

bool is_operator(char c) {
    return c == '*' || c == '+' || c == '-' || c == '/';
}

TokenType punctuator_token(char punctuator) {
    switch (punctuator) {
        case '{': return TOKEN_LBRACE;
        case '}': return TOKEN_RBRACE;
        case '(': return TOKEN_LPAREN;
        case ')': return TOKEN_RPAREN;
        case ';': return TOKEN_SEMICOLON;
        // case '+': return TOKEN_PLUS;
        // case '-': return TOKEN_MINUS;
        // case '*': return TOKEN_STAR;
        // case '/': return TOKEN_DIV;
        default : return TOKEN_UNKNOWN;
    }
}

TokenType single_char_operator(char operator) {
    switch (operator) {
        case '*': return TOKEN_STAR;
        case '+': return TOKEN_PLUS;
        case '-': return TOKEN_MINUS;
        case '/': return TOKEN_DIV;
        default : return TOKEN_UNKNOWN;
    }
}

TokenType operator_token(const char * operator) {
    TokenType token = single_char_operator(operator[0]);
    return token;
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
        case TOKEN_STAR: return "STAR";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_DIV: return "DIV";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NEQ: return "NEQ";
        case TOKEN_GT: return "GT";
        case TOKEN_GE: return "GE";
        case TOKEN_LT: return "LT";
        case TOKEN_LE: return "LE";
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
    idCopy[idLen] = '\0';
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
    numberTextCopy[numberTextLen] = '\0';
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
    punctuatorCopy[punctuatorLen] = '\0';
    token.text = punctuatorCopy;
    token.length = punctuatorLen;
    token.int_value = 0;
    add_token(tokenList, token);
}

int get_operator_len(Token token) {
    // todo need to support both single and double character operators. for now just return 1;
    return 1;
}

void add_operator_token(TokenList * tokenList, const char * operatorText) {
    Token token;
    token.type = operator_token(operatorText);
    int operatorLen = get_operator_len(token);
    char * operatorCopy = malloc(operatorLen + 1);
    memcpy(operatorCopy, operatorText, operatorLen);
    operatorCopy[operatorLen] = '\0';
    token.text = operatorCopy;
    token.length = operatorLen;
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
//            char buffer[2] = { *p, '\0'};
            char * buffer = malloc(2);
            memcpy(buffer, p, 1);
            buffer[1] = '\0';
            add_punctuator_token(tokenList, buffer);

//            formatted_output("PUNCTUATOR:", buffer, tokenType);
            p++;
        }
        else if (is_operator(*p)) {
//            char buffer[2] = { *p, '\0'};
            char * buffer = malloc(2);
            memcpy(buffer, p, 1);
            buffer[1] = '\0';
            add_operator_token(tokenList, buffer);
            p++;
        }
        else if (*p == '=' && *(p+1) == '=') {
            add_token_by_type(tokenList, TOKEN_EQ);
            p += 2;
        }
        else if (*p == '!' && *(p+1) == '=') {
            add_token_by_type(tokenList, TOKEN_NEQ);
            p += 2;
        }
        else if (*p == '>') {
            if (*(p+1) == '=') {
                add_token_by_type(tokenList, TOKEN_GE);
                p += 2;
            }
            else {
                add_token_by_type(tokenList, TOKEN_GT);
                p+= 2;
            }
        }
        else if (*p == '<') {
            if (*(p+1) == '=') {
                add_token_by_type(tokenList, TOKEN_LE);
                p += 2;
            }
            else {
                add_token_by_type(tokenList, TOKEN_LT);
                p+= 2;
            }
        }

    }

    Token eofToken;
    eofToken.type = TOKEN_EOF;
    eofToken.text = '\0';
    eofToken.length = 0;
    add_token(tokenList, eofToken);

}

Token * peek(ParserContext * parserContext) {
    return &parserContext->list->data[parserContext->pos];
}

bool is_current_token(ParserContext * parserContext, TokenType type) {
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

ASTNode * parse_equality_expression(ParserContext * parserContext);
ASTNode * parse_relational_expression(ParserContext * parserContext);
ASTNode * parse_additive_expression(ParserContext * parserContext);
ASTNode * parse_term(ParserContext * parserContext);
ASTNode * parse_expression(ParserContext * parserContext);
ASTNode * parse_factor(ParserContext * parserContext);

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
    if (is_current_token(parserContext, TOKEN_RETURN)) {
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


    // Token * tok = expect_token(parserContext, TOKEN_INT_LITERAL);

    // ASTNode * node = malloc(sizeof(ASTNode));
    // node->type = AST_INT_LITERAL;
    // node->int_value = tok->int_value;
    return root;

}


ASTNode * parse_term(ParserContext * parserContext) {
    ASTNode * root = parse_factor(parserContext);

    while(is_current_token(parserContext, TOKEN_STAR) || is_current_token(parserContext,TOKEN_DIV)) {
        ASTNode * lhs = root;
        Token * op = advance(parserContext);
        ASTNode * rhs = parse_factor(parserContext);
        root = create_binary_op(lhs, op->type, rhs);
    }

    return root;
}

ASTNode * parse_factor(ParserContext * parserContext) {

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
        default:
            printf("Unknown AST Node Type\n");
            break;
    }
}

void emit_return_42(FILE * out) {
    fprintf(out, "section .text\n");
    fprintf(out, "global _start\n");
    fprintf(out, "\n");
    fprintf(out, "_start:\n");
    fprintf(out, "    mov eax, 42\n");
    fprintf(out, "    mov edi, eax\n");
    fprintf(out, "    mov eax, 60\n");
    fprintf(out, "    syscall\n");
}

void emit_header(FILE* out) {
    fprintf(out, "section .text\n");
    fprintf(out, "global main\n");
    fprintf(out, "\n");

}

void emit_binary_op(FILE * out, TokenType op) {
    switch(op) {
        case TOKEN_PLUS:
            fprintf(out, "add eax, ecx\n");
            break;
        case TOKEN_MINUS:
            fprintf(out, "sub eax, ecx\n");
            break;
        case TOKEN_STAR:
            fprintf(out, "imul eax, ecx\n");
            break;
        case TOKEN_DIV:
            fprintf(out, "cdq\n");          // sign-extend eax into edx:eax
            fprintf(out, "idiv ecx\n");
            break;
        default:
            fprintf(stderr, "Unsupported binary operator: %s\n", token_type_name(op));
    }
}
void emit_tree_node(FILE * out, ASTNode * node) {
    if (!node) return;
    switch(node->type) {
        case AST_PROGRAM:
            emit_header(out);
            emit_tree_node(out, node->program.function);
            break;
        case AST_FUNCTION:
            fprintf(out, "%s:\n", node->function.name);
            emit_tree_node(out, node->function.body);
            break;
        case AST_RETURN_STMT:
            emit_tree_node(out, node->return_stmt.expr);
            fprintf(out, "ret\n");
            break;
        case AST_BINARY_OP:
            if (node->binary_op.op == TOKEN_DIV) {
                emit_tree_node(out, node->binary_op.lhs);       // codegen to eval lhs with result in EAX
                fprintf(out, "push rax\n");                     // push lhs result
                emit_tree_node(out, node->binary_op.rhs);       // codegen to eval rhs with result in EAX
                fprintf(out, "mov ecx, eax\n");                 // move denominator to ecx
                fprintf(out, "pop rax\n");                      // restore numerator to eax
                fprintf(out, "cdq\n");
                fprintf(out, "idiv ecx\n");
            }
            else {
                emit_tree_node(out, node->binary_op.lhs);       // codegen to eval lhs with result in EAX
                fprintf(out, "push rax\n");                     // push lhs result
                emit_tree_node(out, node->binary_op.rhs);       // codegen to eval rhs with result in EAX
                fprintf(out, "pop rcx\n");                      // pop lhs to ECX
                emit_binary_op(out, node->binary_op.op);        // emit proper for op
            }
            break;
        case AST_INT_LITERAL:
            fprintf(out, "mov eax, %d\n", node->int_value);
            break;
    }
}

void codegen(ASTNode * program, const char * output_file) {
    FILE * ptr = fopen(output_file, "w");

    emit_tree_node(ptr, program);

    fclose(ptr);
}

const char * read_text_file(const char* filename) {
    FILE * file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char * buffer = malloc(filesize + 1);
    if (!buffer) {
        perror("malloc");
        fclose(file);
        exit(1);
    }

    size_t read_size = fread(buffer, 1, filesize, file);
    if (read_size != filesize) {
        fprintf(stderr, "fread failed\n");
        fclose(file);
        free(buffer);
        exit(1);
    }

    buffer[filesize] = '\0';
    fclose(file);
    return buffer;

}

char* change_extension(const char* source_file, const char* new_ext) {
    const char* dot = strrchr(source_file, '.'); // find last dot
    size_t base_length = (dot) ? (size_t)(dot - source_file) : strlen(source_file);

    size_t new_length = base_length + strlen(new_ext);
    char* out_file = malloc(new_length + 1); // +1 for '\0'
    if (!out_file) {
        perror("malloc");
        exit(1);
    }

    memcpy(out_file, source_file, base_length);
    strcpy(out_file + base_length, new_ext);

    return out_file;
}

int main(int argc, char ** argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source file>", argv[0]);
        return 1;
    }
    const char * program_file = argv[1];
    const char * output_file = change_extension(program_file, ".s");

    const char * sample_program = read_text_file(program_file);

    TokenList tokenList;

    printf("Compiling\n\n%s\n\n", sample_program);

    tokenize(sample_program, &tokenList);

    // output list
    for (int i=0;i<tokenList.count;i++) {
        Token token = tokenList.data[i];
        formatted_output("TOKEN:", token.text, token.type);
    }

    ParserContext parserContext;
    parserContext.list = &tokenList;
    parserContext.pos = 0;

    ASTNode * program = parse_program(&parserContext);
    print_ast(program, 0);

    codegen(program, output_file);

    cleanup_token_list(&tokenList);
    
}                             