int main() {
    int a = 1 ? 10 : 20;
    _print(a);
    int b = 0 ? 10 : 20;
    _print(b);
    int c = (0 ? 1 : 2) ? 3 : 4;
    _print(c);


    int d = a == 10;
    _print(d);

    int e = b == 20;
    _print(e);

    int f = c == 3;
    _print(e);

    int g = d && e && f;
    _print(g);

    return g;
}
