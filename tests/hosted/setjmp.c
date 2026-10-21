#include <setjmp.h>
#include <stdio.h>

jmp_buf environment;

int main(void) {
    int value = setjmp(environment);
    if (value == 0)
        longjmp(environment, 42);
    if (value != 42)
        return 1;
    puts("hosted setjmp/longjmp ok");
    return 0;
}
