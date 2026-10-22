#ifndef __C1_FNMATCH_H
#define __C1_FNMATCH_H
#define FNM_NOMATCH 1
#define FNM_PATHNAME 1
#define FNM_NOESCAPE 2
#define FNM_PERIOD 4
int fnmatch(const char *pattern, const char *text, int flags);
#endif
