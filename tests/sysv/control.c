int main(void) {
    int i, sum = 0;
    for (i = 0; i < 10; i = i + 1)
        sum = sum + i;
    if (sum != 45)
        return 1;
    switch (sum) {
    case 45:
        return 0;
    default:
        return 2;
    }
}
