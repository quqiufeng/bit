/* Entry point for C1-compiled stage1/stage2 executables.  The weak empty
   __c1_init is overridden by a strong definition in an object that has static
   initializers. */
.text
.weak __c1_init
.type __c1_init,@function
__c1_init:
    ret

.globl _start
.type _start,@function
_start:
    xor %rbp, %rbp
    and $-16, %rsp
    sub $32, %rsp
    call __c1_init
    add $32, %rsp
    sub $32, %rsp
    call __c1_start
    mov %eax, %edi
    mov $60, %eax
    syscall
    hlt

.section .note.GNU-stack,"",@progbits
