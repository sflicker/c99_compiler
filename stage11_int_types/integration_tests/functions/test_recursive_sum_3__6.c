int sum_up(int n) {
    _print(n);
    if (n == 0) {
        return 0;
    }
    return n + sum_up(n - 1);
}

int main() {
    return sum_up(3);
}

