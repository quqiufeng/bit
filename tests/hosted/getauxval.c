#include <stdio.h>
#include <sys/auxv.h>

int main(void) {
    unsigned long page = getauxval(AT_PAGESZ);
    if (page < 4096 || (page & (page - 1)) != 0)
        return 1;
    puts("hosted getauxval ok");
    return 0;
}
