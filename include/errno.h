#ifndef __C1_ERRNO_H
#define __C1_ERRNO_H
int *__errno_location(void);
#define errno (*__errno_location())
#define EPERM 1
#define ENOENT 2
#define EINTR 4
#define EIO 5
#define ENOMEM 12
#define EACCES 13
#define EINVAL 22
#define ERANGE 34
#endif
