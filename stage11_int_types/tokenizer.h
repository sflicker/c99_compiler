#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"
#include "tokenizer_context.h"



void tokenize(TokenizerContext * tokenizerContext, TokenList * tokenList);
//bool is_keyword(const char * word);

#endif