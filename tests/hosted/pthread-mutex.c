#include <pthread.h>
#include <stdio.h>

int main(void) {
    pthread_mutex_t mutex;
    if (pthread_mutex_init(&mutex, 0) != 0)
        return 1;
    if (pthread_mutex_lock(&mutex) != 0)
        return 2;
    if (pthread_mutex_trylock(&mutex) == 0)
        return 3;
    if (pthread_mutex_unlock(&mutex) != 0)
        return 4;
    if (pthread_mutex_destroy(&mutex) != 0)
        return 5;
    puts("hosted pthread mutex ok");
    return 0;
}
