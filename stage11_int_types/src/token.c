#include <stdlib.h>
#include <string.h>

#include "list_util.h"
#include "token.h"
#include "util.h"

// void init_token_list(TokenList * list) {
//     list->capacity = 16;
//     list->count = 0;
//     list->data = malloc(sizeof(Token) * list->capacity);
// }

void init_token_list(tokenlist * list) {
    list->head = NULL;
    list->tail = NULL;
}

// void add_token(TokenList * list, Token token) {
//     if (list->count >= list->capacity) {
//         list->capacity *= 2;
//         list->data = realloc(list->data, sizeof(Token) * list->capacity);
//     }
//     list->data[list->count++] = token;
// }

void add_token(tokenlist * tokens, Token * token) {

    tokenlist_append(tokens, token);


//    TokenData * tokenData = malloc(sizeof(TokenData));
    // tokenData->token = token;
    // tokenData->next = NULL;
    // if (tokenList->head == NULL) {
    //     tokenList->head = tokenData;
    //     tokenList->tail = tokenData;
    // }
    // tokenList->tail->next = tokenData;
    // tokenList->tail = tokenData;

    // TokenList *curr = list->next;
    // TokenList * newTokenList = malloc(sizeof(TokenList));
    // newTokenList->data = token;
    // newTokenList
}

Token * make_token(TokenType type, const char * text, int line, int col) {
    Token * token = malloc(sizeof(Token));
    token->type = type;
    token->text = strdup(text);
    token->line = line;
    token->col = col;
    return token;
}

void cleanup_token_list(tokenlist * tokens) {
    // for (int i=0;i<tokenList->count;i++) {
    //     free((void*)tokenList->data[i].text);
    //}

    for (tokenlist_node * node = tokens->head; node; node = node->next) {

    // TokenData * tokenData = tokens->head;
    // while(tokenData) {
        //Token * token = tokenData->token;
        Token * token = node->value;
        free((void*)token->text);
//        TokenData * curr = tokenData;
//        tokenData = tokenData->next;
        free(token);
    }

}

void free_token(Token * token) {
    free(token->text);
    free(token);
}