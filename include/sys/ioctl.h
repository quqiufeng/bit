#ifndef __C1_SYS_IOCTL_H
#define __C1_SYS_IOCTL_H
#define FIONREAD 0x541B
#define TIOCGWINSZ 0x5413
int ioctl(int descriptor, unsigned long request, ...);
#endif
