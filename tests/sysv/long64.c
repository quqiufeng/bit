long twice(long value) {
    return value + value;
}

int main(void) {
    long value = 0x100000000;
    if (sizeof(long) != 8)
        return 1;
    if (sizeof(int) != 4)
        return 2;
    if (sizeof(void *) != 8)
        return 3;
    if (twice(value) != 0x200000000)
        return 4;
    return 0;
}
