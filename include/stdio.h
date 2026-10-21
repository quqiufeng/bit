#ifndef __C1_STDIO_H
#define __C1_STDIO_H
#include <stddef.h>
#include <stdarg.h>
int printf(const char *format, ...);
int vprintf(const char *format, va_list ap);
int sprintf(char *text, const char *format, ...);
int snprintf(char *text, size_t size, const char *format, ...);
int puts(const char *text);
int putchar(int c);
#endif
