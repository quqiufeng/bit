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
typedef union {
    char __size[48];
    long long __align;
} pthread_cond_t;
typedef union {
    char __size[56];
    long long __align;
} pthread_rwlock_t;
typedef int pthread_once_t;
#define PTHREAD_ONCE_INIT 0
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
int pthread_cond_init(pthread_cond_t *condition, const void *attributes);
int pthread_cond_destroy(pthread_cond_t *condition);
int pthread_cond_wait(pthread_cond_t *condition, pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t *condition);
int pthread_cond_broadcast(pthread_cond_t *condition);
int pthread_rwlock_init(pthread_rwlock_t *lock, const void *attributes);
int pthread_rwlock_destroy(pthread_rwlock_t *lock);
int pthread_rwlock_rdlock(pthread_rwlock_t *lock);
int pthread_rwlock_wrlock(pthread_rwlock_t *lock);
int pthread_rwlock_unlock(pthread_rwlock_t *lock);
int pthread_once(pthread_once_t *once, void (*function)(void));
#endif
