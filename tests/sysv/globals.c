static int value = 40;

static int bump(void) {
    return value + 2;
}

int main(void) {
    value = value + 1;
    return bump() == 43 ? 0 : 1;
}
