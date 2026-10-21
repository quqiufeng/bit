#include <stdio.h>
#include <stdlib.h>

static int compare(const void *left, const void *right) {
    int a = *(const int *)left;
    int b = *(const int *)right;
    return a - b;
}

int main(void) {
    int values[5];
    int key = 4;
    int *found;
    values[0] = 5;
    values[1] = 1;
    values[2] = 4;
    values[3] = 2;
    values[4] = 3;
    qsort(values, 5, sizeof(int), compare);
    if (values[0] != 1 || values[1] != 2 || values[2] != 3 || values[3] != 4 || values[4] != 5)
        return 1;
    found = (int *)bsearch(&key, values, 5, sizeof(int), compare);
    if (!found || *found != 4)
        return 2;
    puts("hosted qsort/bsearch callback ok");
    return 0;
}
