void set(int * a) {
    *a = 42;
}

int main() {
    int a;
    set(&a);
    return a;
}

