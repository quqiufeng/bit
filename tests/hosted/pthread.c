#include <pthread.h>
#include <stdio.h>

static void *worker(void *argument) {
    long value = (long)argument;
    return (void *)(value + 2);
}

int main(void) {
    pthread_t thread;
    void *result;
    if (pthread_create(&thread, (pthread_attr_t *)0, worker, (void *)40) != 0)
        return 1;
    if (pthread_join(thread, &result) != 0)
        return 2;
    if ((long)result != 42)
        return 3;
    puts("hosted pthread callback ok");
    return 0;
}
