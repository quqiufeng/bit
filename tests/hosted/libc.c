#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char *text = malloc(16);
    if (!text)
        return 1;
    text[0] = 'o';
    text[1] = 'k';
    text[2] = 0;
    if (strlen(text) != 2)
        return 2;
    puts("hosted malloc/strlen/free ok");
    free(text);
    return 0;
}
