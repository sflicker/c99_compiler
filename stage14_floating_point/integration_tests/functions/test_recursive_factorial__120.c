//
// Created by scott on 6/24/25.
//
int fact(int n) {
    if (n <= 1) return 1;
    return n * fact(n - 1);
}

int main() {
    return fact(5);         // 120
}