/* C source Tokenizer Demo */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "tokenizer.h"
#include "keywords.h"

#define MAX_OUTPUT_SIZE 8192






void tokenize(const char* code) {
    const char* p = code;

    while(*p) {
        if (isspace(*p)) {
            p++;
        }
        else if (isalpha(*p) || *p == '_') {
            // idenfier or keyword
            char buffer[128];
            int i=0;
            while(isalnum(*p) || *p == '_') {
                buffer[i++] = *p++;
            }
            buffer[i] = '\0';

            if (is_keyword(buffer)) {
                printf("KEYWORD: %s\n", buffer);
            } else {
                printf("IDENTIFIER: %s\n", buffer);
            }
        }
        else if (isdigit(*p)) {
            // number
            char buffer[128];
            int i=0;
            while(isdigit(*p)) {
                buffer[i++] = *p++;
            }
            buffer[i++] = '\0';
            printf("NUMBER: %s\n", buffer);
        }
        else if (*p == '"') {
            char buffer[256];
            int i=0;
            buffer[i++] = *p++;
            while(*p && *p != '"') {
                if (*p == '\\') buffer[i++] = *p++;
                buffer[i++] = *p++;
            }
            if (*p == '"') buffer[i++] = *p++;
            buffer[i] = '\0';
            printf("STRING: %s\n", buffer);
        }
        else if (*p == '/' && *(p+1) == '/') {
            // single line comment
            while (*p && *p != '\n') p++;
        }
        else if (*p == '/' && *(p+1) == '*') {
            p += 2;
            while( *p && !(*p == '*' && *(p+1) == '/')) p++;
            if (*p) p += 2;
        }
        else {
            printf("SYMBOL: %c\n", *p++);
        }
    }
}
