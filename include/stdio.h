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
typedef struct __c1_FILE FILE;
FILE *fopen(const char *path, const char *mode);
int fclose(FILE *file);
char *fgets(char *text, int size, FILE *file);
int fputs(const char *text, FILE *file);
#endif
