#ifndef __C1_STDDEF_H
#define __C1_STDDEF_H
typedef unsigned long size_t;
typedef long ptrdiff_t;
#ifndef __C1_WCHAR_T_DEFINED
#define __C1_WCHAR_T_DEFINED
typedef int wchar_t;
#endif
#define NULL ((void *)0)
#define offsetof(type, member) __offsetof(type, member)
#endif
