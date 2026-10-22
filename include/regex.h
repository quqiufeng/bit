#ifndef __C1_REGEX_H
#define __C1_REGEX_H
#include <stddef.h>
typedef struct {
    int rm_so;
    int rm_eo;
} regmatch_t;
typedef struct {
    char __data[64];
} regex_t;
#define REG_EXTENDED 1
#define REG_ICASE 2
#define REG_NOSUB 4
#define REG_NEWLINE 8
#define REG_NOTBOL 1
#define REG_NOTEOL 2
#define REG_NOMATCH 1
int regcomp(regex_t *expression, const char *pattern, int flags);
int regexec(const regex_t *expression, const char *text, size_t count,
            regmatch_t matches[], int flags);
size_t regerror(int error, const regex_t *expression, char *buffer, size_t size);
void regfree(regex_t *expression);
#endif
