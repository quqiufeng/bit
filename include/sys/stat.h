#ifndef __C1_SYS_STAT_H
#define __C1_SYS_STAT_H
#include <sys/types.h>
#ifndef __C1_TIMESPEC_DEFINED
#define __C1_TIMESPEC_DEFINED
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
#endif
struct stat {
    dev_t st_dev;
    ino_t st_ino;
    nlink_t st_nlink;
    mode_t st_mode;
    uid_t st_uid;
    gid_t st_gid;
    int __pad0;
    dev_t st_rdev;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    long __reserved[3];
};
int stat(const char *path, struct stat *value);
int fstat(int fd, struct stat *value);
int mkdir(const char *path, mode_t mode);
int chmod(const char *path, mode_t mode);
#define S_IFMT 0170000
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif
