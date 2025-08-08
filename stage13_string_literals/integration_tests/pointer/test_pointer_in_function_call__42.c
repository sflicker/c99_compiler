//
// Created by scott on 7/15/25.
//

int func(int *b) {
    *b = 42;
    return 0;        // currently not supporting void functions so just return 0 and don't use it in caller
}


int main() {
    int a = 10;
    int dummy = func(&a);      // assign return to a dummy variable which will be ignored.
    return a;
}
