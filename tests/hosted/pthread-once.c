#include <pthread.h>
#include <stdio.h>

static pthread_once_t once = PTHREAD_ONCE_INIT;
static int calls;

static void initialize(void) {
    calls++;
}

int main(void) {
    pthread_rwlock_t lock;
    if (pthread_once(&once, initialize) != 0 || pthread_once(&once, initialize) != 0)
        return 1;
    if (calls != 1)
        return 2;
    if (pthread_rwlock_init(&lock, 0) != 0)
        return 3;
    if (pthread_rwlock_wrlock(&lock) != 0)
        return 4;
    if (pthread_rwlock_unlock(&lock) != 0)
        return 5;
    if (pthread_rwlock_rdlock(&lock) != 0)
        return 6;
    if (pthread_rwlock_unlock(&lock) != 0)
        return 7;
    if (pthread_rwlock_destroy(&lock) != 0)
        return 8;
    puts("hosted pthread once/rwlock ok");
    return 0;
}
