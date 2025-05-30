#include <stdio.h>
#include <string.h>
#include "list_util.h"

void dummy_free(int i) {

}

void free_charptr(char * p) {
    free(p);
}

char * strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char * copy = malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

typedef struct {
    char * text;
    int type;
} Token;

void free_token(Token * token) {
    free(token->text);
}

DEFINE_LINKED_LIST(int, intlist);
DEFINE_LINKED_LIST(char*, stringlist);
DEFINE_LINKED_LIST(Token*, tokenlist);

int main() {
	intlist nums = {0};
//    intlist_init(&nums);

    intlist_append(&nums, 10);
    intlist_append(&nums, 20);
    intlist_append(&nums, 30);

    printf("List Count: %d\n", nums.count);

    printf("iterating with for\n");
    for (intlist_node * n = nums.head; n;n = n->next) {
        int num = n->value;
        printf("num: %d\n", num);
    }

    printf("iterating with while and cursor\n");
    intlist_cursor cur;
    intlist_cursor_init(&cur, &nums);
    while(intlist_cursor_has_next(&cur)) {
        int num = intlist_cursor_next(&cur);
        printf("num: %d\n", num);
    }

    intlist_free(&nums);

    printf("After free, count = %d\n", nums.count);

    stringlist names = {0};
    stringlist_init(&names, free_charptr);

    char * s1 = strdup("Alice");
    char * s2 = strdup("Bob");
    char * s3 = strdup("Charlie");

    stringlist_append(&names, s1);
    stringlist_append(&names, s2);
    stringlist_append(&names, s3);

    printf("Names:\n");
    for (stringlist_node * node = names.head; node; node = node->next) {
        printf(" - %s\n", node->value);
    }

//    stringlist_free(&names, (void(*)(char*)) free);
    stringlist_free(&names);

    Token * token = malloc(sizeof(Token));
    token->text = strdup("Sample");
    token->type = 1;

    tokenlist tokens;
    tokenlist_init(&tokens, free_token);
    tokenlist_append(&tokens, token);
    printf("Tokens:\n");
    for( tokenlist_node * node = tokens.head;node;node = node->next) {
        Token * tok = node->value;
        printf("Token: %d, %s\n", tok->type, tok->text);
    }
    tokenlist_free(&tokens);

    return 0;
}

