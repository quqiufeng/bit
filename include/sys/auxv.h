#ifndef __C1_SYS_AUXV_H
#define __C1_SYS_AUXV_H
#define AT_NULL 0
#define AT_PAGESZ 6
#define AT_HWCAP 16
#define AT_CLKTCK 17
#define AT_HWCAP2 26
unsigned long getauxval(unsigned long type);
#endif
