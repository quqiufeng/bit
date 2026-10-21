#ifndef __C1_STRING_H
#define __C1_STRING_H
#include <stddef.h>
size_t strlen(const char *text);
void *memcpy(void *destination, const void *source, size_t count);
void *memmove(void *destination, const void *source, size_t count);
void *memset(void *destination, int value, size_t count);
int strcmp(const char *left, const char *right);
#endif
