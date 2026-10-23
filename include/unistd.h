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
char *getcwd(char *buffer, size_t size);
int chdir(const char *path);
int access(const char *path, int mode);
int isatty(int fd);
ssize_t readlink(const char *path, char *buffer, size_t size);
int pipe(int fds[2]);
int dup(int fd);
int dup2(int old_fd, int new_fd);
pid_t fork(void);
uid_t getuid(void);
gid_t getgid(void);
int execve(const char *path, char *const arguments[], char *const environment[]);
int execv(const char *path, char *const arguments[]);
int execvp(const char *file, char *const arguments[]);
void _exit(int status);
unsigned int sleep(unsigned int seconds);
#endif
