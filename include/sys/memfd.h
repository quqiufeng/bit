#ifndef __C1_SYS_MEMFD_H
#define __C1_SYS_MEMFD_H
#define MFD_CLOEXEC 1
#define MFD_ALLOW_SEALING 2
#define MFD_HUGETLB 4
int memfd_create(const char *name, unsigned int flags);
#endif
