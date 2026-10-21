typedef int Binary(int, int);

int add(int a, int b) {
    return a + b;
}

int apply(Binary *function, int a, int b) {
    return function(a, b);
}

int main(void) {
    int value = 40;
    int *pointer = &value;
    Binary *function = add;
    if (*pointer != 40)
        return 1;
    return apply(function, *pointer, 2) == 42 ? 0 : 2;
}
