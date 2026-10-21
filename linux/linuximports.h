#ifndef C1_LINUX_IMPORTS_H
#define C1_LINUX_IMPORTS_H
/* Linux runtime entry points used by the host compiler.  The names mirror
   the Win32 imports in the upstream compiler so the ported compiler body
   stays textually close to its origin.  They are ordinary ELF symbols, not
   import-table slots. */
extern void __c1_linux_GetStdHandle(void);
extern void __c1_linux_ReadFile(void);
extern void __c1_linux_WriteFile(void);
extern void __c1_linux_CreateFileA(void);
extern void __c1_linux_CloseHandle(void);
extern void __c1_linux_GetEnvironmentVariableA(void);
extern void __c1_linux_GetLocalTime(void);
extern void __c1_linux_GetProcessHeap(void);
extern void __c1_linux_HeapAlloc(void);
extern void __c1_linux_HeapFree(void);
extern void __c1_linux_IsDBCSLeadByteEx(void);
extern int __c0_getchar(void);
#endif
