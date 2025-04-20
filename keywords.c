#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


#include "keywords.h"


const char * keywords[] = {
    "int", "char", "if", "else", "while", "return", "for", "void", "float", "double", "struct"
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
