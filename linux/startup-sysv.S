/* Entry point for SysV executables built by gcc or by C1 itself. */
.text
.weak __c1_init
.type __c1_init,@function
__c1_init:
    ret

.globl _start
.type _start,@function
_start:
    xor %rbp, %rbp
    mov %rsp, %r12
    and $-16, %rsp
    call __c1_init
    mov (%r12), %rdi
    lea 8(%r12), %rsi
    lea 16(%r12,%rdi,8), %rdx
    call __c1_start
    mov %eax, %edi
    mov $60, %eax
    syscall
    hlt

.section .note.GNU-stack,"",@progbits
