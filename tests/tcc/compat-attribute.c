__extension__ __attribute__((unused)) int static_check;

struct __attribute__((packed)) S {
    char a;
    int b;
};

struct __attribute__((aligned(16))) A {
    char c;
};

struct B {
    char c;
    int x __attribute__((aligned(16)));
};

void *bit_exit(int code) __attribute__((noreturn));

int main(void) {
    if (sizeof(struct S) != 5)
        return 1;
    if (sizeof(struct A) != 16)
        return 2;
    if (sizeof(struct B) != 32)
        return 3;
    return 0;
}
