#ifndef __C1_UNISTD_H
#define __C1_UNISTD_H
#include <stddef.h>
#include <sys/types.h>
ssize_t read(int fd, void *buffer, size_t count);
ssize_t write(int fd, const void *buffer, size_t count);
int close(int fd);
off_t lseek(int fd, off_t offset, int whence);
int unlink(const char *path);
int getpid(void);
pid_t fork(void);
int execve(const char *path, char *const arguments[], char *const environment[]);
void _exit(int status);
unsigned int sleep(unsigned int seconds);
#endif
