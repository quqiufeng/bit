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
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
int fprintf(FILE *file, const char *format, ...);
int vfprintf(FILE *file, const char *format, va_list ap);
int vsnprintf(char *text, size_t size, const char *format, va_list ap);
int sscanf(const char *text, const char *format, ...);
int fscanf(FILE *file, const char *format, ...);
int vsscanf(const char *text, const char *format, va_list ap);
int fflush(FILE *file);
int fclose(FILE *file);
int fileno(FILE *file);
int ferror(FILE *file);
int feof(FILE *file);
void clearerr(FILE *file);
int fseek(FILE *file, long offset, int whence);
long ftell(FILE *file);
void rewind(FILE *file);
int ungetc(int c, FILE *file);
int getc(FILE *file);
int putc(int c, FILE *file);
void perror(const char *text);
FILE *freopen(const char *path, const char *mode, FILE *file);

FILE *fopen(const char *path, const char *mode);
FILE *fdopen(int fd, const char *mode);
int fileno(FILE *file);
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
