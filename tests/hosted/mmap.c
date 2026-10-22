#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

int main(void) {
    char *memory;
    memory = mmap(0, 4096, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED)
        return 1;
    strcpy(memory, "mapped");
    if (strcmp(memory, "mapped") != 0)
        return 2;
    if (mprotect(memory, 4096, PROT_READ) != 0)
        return 3;
    if (munmap(memory, 4096) != 0)
        return 4;
    puts("hosted mmap ok");
    return 0;
}
