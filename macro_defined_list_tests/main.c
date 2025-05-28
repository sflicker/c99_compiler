#include <stdio.h>
#include "list_util.h"

DEFINE_LINKED_LIST(int, intlist);

int main() {
	intlist nums = {0};
//    intlist_init(&nums);

    intlist_append(&nums, 10);
    intlist_append(&nums, 20);
    intlist_append(&nums, 30);

    printf("List Count: %d\n", nums.count);

    return 0;
}

