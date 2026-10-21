#ifndef __C1_STDLIB_H
#define __C1_STDLIB_H
#include <stddef.h>
void *malloc(size_t size);
void free(void *memory);
void exit(int status);
int atoi(const char *text);
#endif
