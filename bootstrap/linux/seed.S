.intel_syntax noprefix
.global _start
_start:
    sub rsp, 16
    xor r12d, r12d
    xor r13d, r13d
.read:
    xor eax, eax
    xor edi, edi
    mov rsi, rsp
    mov edx, 1
    syscall
    test rax, rax
    jle .done
    movzx eax, byte ptr [rsp]
    cmp al, 59
    je .comment
    cmp al, 32
    je .read
    cmp al, 9
    je .read
    cmp al, 10
    je .read
    cmp al, 13
    je .read
    mov edx, eax
    cmp dl, 48
    jb .bad
    cmp dl, 57
    jbe .digit
    cmp dl, 65
    jb .bad
    cmp dl, 70
    jbe .upper
    cmp dl, 97
    jb .bad
    cmp dl, 102
    ja .bad
    sub edx, 87
    jmp .got
.upper:
    sub edx, 55
    jmp .got
.digit:
    sub edx, 48
.got:
    test r12d, r12d
    jnz .second
    mov r13d, edx
    mov r12d, 1
    jmp .read
.second:
    shl r13d, 4
    or r13d, edx
    mov byte ptr [rsp], r13b
    mov eax, 1
    mov edi, 1
    mov rsi, rsp
    mov edx, 1
    syscall
    xor r12d, r12d
    jmp .read
.comment:
    xor eax, eax
    xor edi, edi
    mov rsi, rsp
    mov edx, 1
    syscall
    test rax, rax
    jle .done
    cmp byte ptr [rsp], 10
    jne .comment
    jmp .read
.bad:
    mov eax, 60
    mov edi, 1
    syscall
.done:
    mov eax, 60
    xor edi, edi
    syscall
