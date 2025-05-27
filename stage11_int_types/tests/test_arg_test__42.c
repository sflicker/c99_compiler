

int func(char a, short b, int c, long d) {
    _print(a);  _assert(a == 9);
    _print(b);  _assert(b == 10);
    _print(c);  _assert(c == 11);
    _print(d);  _assert(d == 12);
    return a+b+c+d;
}

int main() {
	int e = func(9,10,11,12);
    _print(e); _assert(e == 42);
    return e;
}
