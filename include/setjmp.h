#ifndef __C1_SETJMP_H
#define __C1_SETJMP_H
typedef long jmp_buf[25];
int setjmp(jmp_buf environment);
void longjmp(jmp_buf environment, int value);
#endif
