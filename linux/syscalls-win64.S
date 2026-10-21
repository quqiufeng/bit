/* Microsoft x64 ABI syscall stubs for C1-compiled stage1/stage2 runtime. */
.text

.globl __c1_win64_read
.type __c1_win64_read,@function
__c1_win64_read:
    mov %rcx, %rdi
    mov %rdx, %rsi
    mov %r8, %rdx
    mov $0, %eax
    syscall
    ret

.globl __c1_win64_write
.type __c1_win64_write,@function
__c1_win64_write:
    mov %rcx, %rdi
    mov %rdx, %rsi
    mov %r8, %rdx
    mov $1, %eax
    syscall
    ret

.globl __c1_win64_open
.type __c1_win64_open,@function
__c1_win64_open:
    mov %rcx, %rdi
    mov %rdx, %rsi
    mov %r8, %rdx
    mov $2, %eax
    syscall
    ret

.globl __c1_win64_close
.type __c1_win64_close,@function
__c1_win64_close:
    mov %rcx, %rdi
    mov $3, %eax
    syscall
    ret

.globl __c1_win64_mmap
.type __c1_win64_mmap,@function
__c1_win64_mmap:
    mov %rcx, %rdi
    mov %rdx, %rsi
    mov %r8, %rdx
    mov %r9, %r10
    mov 40(%rsp), %r8
    mov 48(%rsp), %r9
    mov $9, %eax
    syscall
    ret

.globl __c1_win64_munmap
.type __c1_win64_munmap,@function
__c1_win64_munmap:
    mov %rcx, %rdi
    mov %rdx, %rsi
    mov $11, %eax
    syscall
    ret

.globl __c1_win64_exit
.type __c1_win64_exit,@function
__c1_win64_exit:
    mov %rcx, %rdi
    mov $60, %eax
    syscall
    hlt

.section .note.GNU-stack,"",@progbits
