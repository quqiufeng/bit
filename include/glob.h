#ifndef __C1_GLOB_H
#define __C1_GLOB_H
#include <stddef.h>
typedef struct {
    size_t gl_pathc;
    char **gl_pathv;
    size_t gl_offs;
    int gl_flags;
    void *__f1;
    void *__f2;
    void *__f3;
    void *__f4;
    void *__f5;
} glob_t;
#define GLOB_ERR 1
#define GLOB_MARK 2
#define GLOB_NOSORT 4
#define GLOB_DOOFFS 8
#define GLOB_NOCHECK 16
#define GLOB_APPEND 32
#define GLOB_NOESCAPE 64
#define GLOB_PERIOD 128
#define GLOB_NOMATCH 3
int glob(const char *pattern, int flags, int (*error_function)(const char *, int), glob_t *paths);
void globfree(glob_t *paths);
#endif
