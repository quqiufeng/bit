#include <stdint.h>
#include <stdio.h>
#include <sys/eventfd.h>
#include <unistd.h>

int main(void) {
    uint64_t value = 42;
    uint64_t result = 0;
    int descriptor = eventfd(0, EFD_CLOEXEC);
    if (descriptor < 0)
        return 1;
    if (write(descriptor, &value, sizeof(value)) != (long)sizeof(value))
        return 2;
    if (read(descriptor, &result, sizeof(result)) != (long)sizeof(result))
        return 3;
    close(descriptor);
    if (result != 42)
        return 4;
    puts("hosted eventfd ok");
    return 0;
}
