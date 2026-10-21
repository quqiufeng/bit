/* SysV AMD64 syscall stubs for the gcc-built stage0 runtime. */
.text

.globl __c1_sysv_read
.type __c1_sysv_read,@function
__c1_sysv_read:
    mov $0, %eax
    syscall
    ret

.globl __c1_sysv_write
.type __c1_sysv_write,@function
__c1_sysv_write:
    mov $1, %eax
    syscall
    ret

.globl __c1_sysv_open
.type __c1_sysv_open,@function
__c1_sysv_open:
    mov $2, %eax
    syscall
    ret

.globl __c1_sysv_close
.type __c1_sysv_close,@function
__c1_sysv_close:
    mov $3, %eax
    syscall
    ret

.globl __c1_sysv_mmap
.type __c1_sysv_mmap,@function
__c1_sysv_mmap:
    mov %rcx, %r10
    mov $9, %eax
    syscall
    ret

.globl __c1_sysv_munmap
.type __c1_sysv_munmap,@function
__c1_sysv_munmap:
    mov $11, %eax
    syscall
    ret

.globl __c1_sysv_exit
.type __c1_sysv_exit,@function
__c1_sysv_exit:
    mov $60, %eax
    syscall
    hlt

.globl __c1_sysv_lseek
.type __c1_sysv_lseek,@function
__c1_sysv_lseek:
    mov $8, %eax
    syscall
    ret

.section .note.GNU-stack,"",@progbits
