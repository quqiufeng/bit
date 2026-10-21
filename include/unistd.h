#ifndef __C1_UNISTD_H
#define __C1_UNISTD_H
#include <stddef.h>
typedef long ssize_t;
typedef long off_t;
ssize_t read(int fd, void *buffer, size_t count);
ssize_t write(int fd, const void *buffer, size_t count);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
int unlink(const char *path);
int getpid(void);
#endif
