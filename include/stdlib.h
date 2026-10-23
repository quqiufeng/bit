#ifndef __C1_STDLIB_H
#define __C1_STDLIB_H
#include <stddef.h>
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *memory, size_t size);
void free(void *memory);
void exit(int status);
void abort(void);
int atexit(void (*function)(void));
char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int atoi(const char *text);
long strtol(const char *text, char **end, int base);
unsigned long strtoul(const char *text, char **end, int base);
long long strtoll(const char *text, char **end, int base);
unsigned long long strtoull(const char *text, char **end, int base);
double strtod(const char *text, char **end);
void qsort(void *base, unsigned long count, unsigned long size,
           int (*compare)(const void *, const void *));
void *bsearch(const void *key, const void *base, unsigned long count, unsigned long size,
              int (*compare)(const void *, const void *));
int abs(int value);
long labs(long value);
long long llabs(long long value);
typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;
div_t div(int numerator, int denominator);
ldiv_t ldiv(long numerator, long denominator);
int rand(void);
void srand(unsigned int seed);
int system(const char *command);
#endif
