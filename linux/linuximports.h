#ifndef C1_LINUX_IMPORTS_H
#define C1_LINUX_IMPORTS_H
extern long long __c1_linux_GetStdHandle(int);
extern long long __c1_linux_GetProcessHeap(void);
extern void *__c1_linux_HeapAlloc(long long, unsigned int, unsigned long long);
extern int __c1_linux_HeapFree(long long, unsigned int, void *);
extern int __c1_linux_ReadFile(long long, char *, int, int *, void *);
extern int __c1_linux_WriteFile(long long, char *, int, int *, void *);
extern long long __c1_linux_CreateFileA(char *, unsigned int, int, void *, int, int, long long);
extern int __c1_linux_CloseHandle(long long);
extern unsigned int __c1_linux_GetEnvironmentVariableA(char *, char *, unsigned int);
extern void __c1_linux_GetLocalTime(unsigned short *);
extern int __c1_linux_IsDBCSLeadByteEx(unsigned int, unsigned char);
extern int __c0_getchar(void);
extern int __c0_putchar(int);
#endif
