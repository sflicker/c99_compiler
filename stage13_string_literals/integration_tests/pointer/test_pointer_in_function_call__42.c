//
// Created by scott on 7/15/25.
//

int func(int *b) {
    *b = 42;
    return 0;
}


int main() {
    int a = 10;
    func(&a);
    return a;
}