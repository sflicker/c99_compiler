#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "token.h"
#include "tokenizer_context.h"



tokenlist * tokenize(const char * text);

Token * match_keyword(TokenizerContext * ctx, const char * text);

#endif