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
int fgetc(FILE *file);
int fputc(int c, FILE *file);
size_t fread(void *buffer, size_t size, size_t count, FILE *file);
size_t fwrite(const void *buffer, size_t size, size_t count, FILE *file);
int fseek(FILE *file, long offset, int whence);
long ftell(FILE *file);
void rewind(FILE *file);
int feof(FILE *file);
int ferror(FILE *file);
int remove(const char *path);
int rename(const char *old_path, const char *new_path);
#endif
