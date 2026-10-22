#ifndef __C1_SYS_RANDOM_H
#define __C1_SYS_RANDOM_H
#include <stddef.h>
#define GRND_NONBLOCK 1
#define GRND_RANDOM 2
long getrandom(void *buffer, size_t length, unsigned int flags);
#endif
