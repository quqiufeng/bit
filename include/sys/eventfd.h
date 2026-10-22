#ifndef __C1_SYS_EVENTFD_H
#define __C1_SYS_EVENTFD_H
#include <stdint.h>
#define EFD_CLOEXEC 0x80000
#define EFD_NONBLOCK 0x800
#define EFD_SEMAPHORE 1
int eventfd(unsigned int initial, int flags);
#endif
