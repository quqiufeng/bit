int puts(char *text);

int value = 40;

int main(void) {
    value = value + 2;
    puts("hello from C1");
    return value == 42 ? 0 : 1;
}
