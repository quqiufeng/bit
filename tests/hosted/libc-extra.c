#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *path = "/tmp/c1-hosted-extra.txt";
    char buffer[16];
    char text[16];
    FILE *file;
    div_t quotient;
    if (abs(-42) != 42 || labs(-42L) != 42L || llabs(-42LL) != 42LL)
        return 1;
    quotient = div(43, 5);
    if (quotient.quot != 8 || quotient.rem != 3)
        return 2;
    strcpy(text, "ab");
    strcat(text, "cd");
    if (strcmp(text, "abcd") != 0 || strncmp(text, "abc", 3) != 0)
        return 3;
    if (!strchr(text, 'c') || strstr(text, "bc") == 0)
        return 4;
    file = fopen(path, "w+b");
    if (!file)
        return 5;
    if (fwrite("hello", 1, 5, file) != 5)
        return 6;
    if (fseek(file, 0, SEEK_SET) != 0 || ftell(file) != 0)
        return 7;
    if (fread(buffer, 1, 5, file) != 5)
        return 8;
    if (fclose(file) != 0)
        return 9;
    buffer[5] = 0;
    if (memcmp(buffer, "hello", 5) != 0)
        return 10;
    srand(1);
    if (rand() < 0)
        return 11;
    remove(path);
    puts("hosted libc extra ok");
    return 0;
}
