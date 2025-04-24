#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "macros.h"

#define MAX_MARCOS 100

typedef struct {
    char name[64];
    char replacement[256];
} Macro;

Macro macros[MAX_MARCOS];
int macro_count = 0;


void add_macro(const char* name, const char* value) {
    if (macro_count >= MAX_MARCOS) return;
    strncpy(macros[macro_count].name, name, sizeof(macros[macro_count].name));
    strncpy(macros[macro_count].replacement, value, sizeof(macros[macro_count].replacement));
}

const char * find_macro(const char * name) {
    for (int i=0;i<macro_count;i++) {
        if (strcmp(macros[i].name, name) == 0) {
            return macros[i].replacement;
        }
    }
    return NULL;
}

void remove_macro(const char * name) {
    for (int i=0;i<macro_count;i++) {
        if (strcmp(macros[i].name, name) == 0) {
            macros[i] = macros[--macro_count];
            break;
        }
    }
}