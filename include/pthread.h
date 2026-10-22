#ifndef __C1_PTHREAD_H
#define __C1_PTHREAD_H
typedef unsigned long pthread_t;
typedef union {
    char __size[56];
    long __align;
} pthread_attr_t;
typedef union {
    char __size[40];
    long __align;
} pthread_mutex_t;
#define PTHREAD_MUTEX_INITIALIZER { { 0 } }
int pthread_create(pthread_t *thread, const pthread_attr_t *attributes,
                   void *(*start)(void *), void *argument);
int pthread_join(pthread_t thread, void **result);
void pthread_exit(void *result);
pthread_t pthread_self(void);
int pthread_mutex_init(pthread_mutex_t *mutex, const void *attributes);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
#endif
