#ifndef __C1_STDLIB_H
#define __C1_STDLIB_H
#include <stddef.h>
void *malloc(size_t size);
void free(void *memory);
void exit(int status);
void abort(void);
int atoi(const char *text);
long strtol(const char *text, char **end, int base);
unsigned long strtoul(const char *text, char **end, int base);
long long strtoll(const char *text, char **end, int base);
unsigned long long strtoull(const char *text, char **end, int base);
double strtod(const char *text, char **end);
#endif
