#include <dlfcn.h>
#include <stdio.h>

int main(void) {
    void *handle = dlopen("/tmp/c1-hosted-plugin.so", RTLD_LAZY);
    int (*function)(void);
    if (!handle)
        return 1;
    function = (int (*)(void))dlsym(handle, "answer");
    if (!function)
        return 2;
    if (function() != 42)
        return 3;
    dlclose(handle);
    puts("hosted C1 shared object ok");
    return 0;
}
