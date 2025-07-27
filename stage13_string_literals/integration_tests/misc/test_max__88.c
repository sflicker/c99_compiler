int max3(int a, int b, int c) {
    _print(a);
    _print(b);
    _print(c);

    int m = a;
    _print(m);
    if (b > m) m = b;
    _print(m);
    if (c > m) m = c;
    _print(m);

    return m;
}

int main() {
    return max3(42, 17, 88);	// expected: 88
}

