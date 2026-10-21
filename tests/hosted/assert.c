#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(1 == 1);
    assert(sizeof(long) == 8);
    puts("hosted assert ok");
    return 0;
}
