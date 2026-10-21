/* Entry point for the gcc/SysV stage0 executable. */
.text
.globl _start
.type _start,@function
_start:
    xor %rbp, %rbp
    and $-16, %rsp
    call __c1_start
    mov %eax, %edi
    mov $60, %eax
    syscall
    hlt

.section .note.GNU-stack,"",@progbits
