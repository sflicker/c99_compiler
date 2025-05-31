#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"
#include "tokenizer_context.h"



//void tokenize(TokenizerContext * tokenizerContext, tokenlist * tokenList);
tokenlist * tokenize(const char * text);
//bool is_keyword(const char * word);

#endif