#ifndef __C1_PTHREAD_H
#define __C1_PTHREAD_H
typedef unsigned long pthread_t;
typedef union {
    char __size[56];
    long __align;
} pthread_attr_t;
int pthread_create(pthread_t *thread, const pthread_attr_t *attributes,
                   void *(*start)(void *), void *argument);
int pthread_join(pthread_t thread, void **result);
void pthread_exit(void *result);
pthread_t pthread_self(void);
#endif
