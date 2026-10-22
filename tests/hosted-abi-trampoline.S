.text

.macro SAVE_CALLEE
    push %rbp
    mov %rsp, %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15
    sub $8, %rsp
.endm

.macro POLLUTE_CALLEE
    mov $0x1111111111111111, %rbx
    mov $0x2222222222222222, %r12
    mov $0x3333333333333333, %r13
    mov $0x4444444444444444, %r14
    mov $0x5555555555555555, %r15
.endm

.macro CHECK_CALLEE
    movabs $0x1111111111111111, %r11
    cmp %r11, %rbx; jne .abi_fatal
    movabs $0x2222222222222222, %r11
    cmp %r11, %r12; jne .abi_fatal
    movabs $0x3333333333333333, %r11
    cmp %r11, %r13; jne .abi_fatal
    movabs $0x4444444444444444, %r11
    cmp %r11, %r14; jne .abi_fatal
    movabs $0x5555555555555555, %r11
    cmp %r11, %r15; jne .abi_fatal
.endm

.macro CHECK_X87
    push %rax
    fnstsw %ax
    test $0x3800, %ax
    pop %rax
    jne .abi_fatal
.endm

.macro RESTORE_CALLEE
    add $8, %rsp
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp
    ret
.endm

.global call_with_pollution_0
call_with_pollution_0:
    SAVE_CALLEE
    mov %rdi, %r10
    POLLUTE_CALLEE
    mov $0xdeadbeef, %r8
    mov $0xdeadbeef, %r9
    mov $0xdeadbeef, %r11
    call *%r10
    CHECK_CALLEE
    CHECK_X87
    RESTORE_CALLEE

.global call_with_pollution_2
call_with_pollution_2:
    SAVE_CALLEE
    mov %rdi, %r10
    mov %rsi, %rdi
    mov %rdx, %rsi
    POLLUTE_CALLEE
    mov $0xdeadbeef, %r8
    mov $0xdeadbeef, %r9
    mov $0xdeadbeef, %r11
    call *%r10
    CHECK_CALLEE
    CHECK_X87
    RESTORE_CALLEE

.global call_with_pollution_6
call_with_pollution_6:
    SAVE_CALLEE
    mov %rdi, %r10
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    mov %r8, %rcx
    mov %r9, %r8
    mov 16(%rbp), %r9
    POLLUTE_CALLEE
    mov $0xdeadbeef, %r11
    call *%r10
    CHECK_CALLEE
    CHECK_X87
    RESTORE_CALLEE


.global call_with_pollution_7
call_with_pollution_7:
    SAVE_CALLEE
    mov %rdi, %r10
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    mov %r8, %rcx
    mov %r9, %r8
    mov 16(%rbp), %r9
    mov 24(%rbp), %r11
    mov %r11, (%rsp)
    POLLUTE_CALLEE
    mov $0xdeadbeef, %r11
    call *%r10
    CHECK_CALLEE
    CHECK_X87
    RESTORE_CALLEE

.global call_with_pollution_2d
call_with_pollution_2d:
    SAVE_CALLEE
    mov %rdi, %r10
    mov %rsi, %rdi
    mov $1, %rax
    POLLUTE_CALLEE
    mov $0xdeadbeef, %r8
    mov $0xdeadbeef, %r9
    mov $0xdeadbeef, %r11
    call *%r10
    CHECK_CALLEE
    CHECK_X87
    RESTORE_CALLEE

.abi_fatal:
    mov $255, %rdi
    call exit
.size call_with_pollution_0, .-call_with_pollution_0
.size call_with_pollution_2, .-call_with_pollution_2
.size call_with_pollution_6, .-call_with_pollution_6
.size call_with_pollution_2d, .-call_with_pollution_2d
.size call_with_pollution_7, .-call_with_pollution_7
.section .note.GNU-stack,"",@progbits
