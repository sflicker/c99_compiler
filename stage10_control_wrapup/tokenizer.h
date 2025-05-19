#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"



void cleanup_token_list(TokenList * tokenList);
void tokenize(const char* code, TokenList * tokenList);
bool is_keyword(const char * word);

#endif