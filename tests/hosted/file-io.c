#include <stdio.h>
#include <string.h>

int main(void) {
    FILE *file;
    char buffer[32];
    file = fopen("/tmp/c1-hosted-io.txt", "w");
    if (!file)
        return 1;
    if (fputs("hosted file ok\n", file) < 0)
        return 2;
    if (fclose(file) != 0)
        return 3;
    file = fopen("/tmp/c1-hosted-io.txt", "r");
    if (!file)
        return 4;
    if (!fgets(buffer, sizeof(buffer), file))
        return 5;
    fclose(file);
    if (strcmp(buffer, "hosted file ok\n") != 0)
        return 6;
    puts("hosted file I/O ok");
    return 0;
}
