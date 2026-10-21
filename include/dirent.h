#ifndef __C1_DIRENT_H
#define __C1_DIRENT_H
#include <sys/types.h>
typedef struct __c1_DIR DIR;
struct dirent {
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[256];
};
DIR *opendir(const char *path);
struct dirent *readdir(DIR *directory);
int closedir(DIR *directory);
#define DT_UNKNOWN 0
#define DT_REG 8
#define DT_DIR 4
#endif
