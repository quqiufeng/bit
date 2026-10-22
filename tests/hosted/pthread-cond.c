#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t mutex;
static pthread_cond_t condition;
static int ready;

static void *worker(void *argument) {
    pthread_mutex_lock(&mutex);
    while (!ready)
        pthread_cond_wait(&condition, &mutex);
    pthread_mutex_unlock(&mutex);
    return (void *)42;
}

int main(void) {
    pthread_t thread;
    void *result;
    if (pthread_mutex_init(&mutex, 0) != 0)
        return 1;
    if (pthread_cond_init(&condition, 0) != 0)
        return 2;
    if (pthread_create(&thread, 0, worker, 0) != 0)
        return 3;
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);
    if (pthread_join(thread, &result) != 0)
        return 4;
    if ((long)result != 42)
        return 5;
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&mutex);
    puts("hosted pthread cond ok");
    return 0;
}
