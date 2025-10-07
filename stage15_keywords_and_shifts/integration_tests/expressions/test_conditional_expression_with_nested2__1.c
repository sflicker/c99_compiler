int main() {
    int a = 1 ? 10 : 20;
    _print(a);
    int b = 0 ? 10 : 20;
    _print(b);
    int c = (0 ? 1 : 2) ? 3 : 4;
    _print(c);

    int g = (a == 10  && b == 20 && c == 3);
    _print(g);

    return g;
}
