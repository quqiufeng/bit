#ifndef __C1_WCHAR_H
#define __C1_WCHAR_H
#include <stddef.h>
typedef unsigned int wint_t;
size_t wcslen(const wchar_t *text);
wchar_t *wcscpy(wchar_t *destination, const wchar_t *source);
wchar_t *wcsncpy(wchar_t *destination, const wchar_t *source, size_t count);
wchar_t *wcscat(wchar_t *destination, const wchar_t *source);
int wcscmp(const wchar_t *left, const wchar_t *right);
int wcsncmp(const wchar_t *left, const wchar_t *right, size_t count);
wchar_t *wcschr(const wchar_t *text, wchar_t c);
#endif
