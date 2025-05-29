#include <stdio.h>
#include <string.h>
#include "list_util.h"

DEFINE_LINKED_LIST(int, intlist);
DEFINE_LINKED_LIST(char*, stringlist);

char * strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char * copy = malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

void free_charptr(char * p) {
    free(p);
}

int main() {
	intlist nums = {0};
//    intlist_init(&nums);

    intlist_append(&nums, 10);
    intlist_append(&nums, 20);
    intlist_append(&nums, 30);

    printf("List Count: %d\n", nums.count);

    for (intlist_node * n = nums.head; n;n = n->next) {
        int num = n->value;
        printf("num: %d\n", num);
    }

    intlist_free(&nums, NULL);

    printf("After free, count = %d\n", nums.count);

    stringlist names = {0};
    stringlist_init(&names);

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
    stringlist_free(&names, free_charptr);

    return 0;
}

