.intel_syntax noprefix
.section .text

.global stub_GetStdHandle
stub_GetStdHandle:
    mov rax, rcx
    ret

.global stub_ExitProcess
stub_ExitProcess:
    mov rdi, rcx
    mov eax, 60
    syscall
    hlt

.global stub_ReadFile
stub_ReadFile:
    push rdi
    push rsi
    mov r10, r9
    movsxd rdi, ecx
    test rdi, rdi
    jns 1f
    neg rdi
    sub rdi, 10
1:
    mov rsi, rdx
    mov rdx, r8
    xor eax, eax
    syscall
    test rax, rax
    js 2f
    test r10, r10
    jz 3f
    mov dword ptr [r10], eax
3:
    mov eax, 1
    pop rsi
    pop rdi
    ret
2:
    test r10, r10
    jz 4f
    mov dword ptr [r10], 0
4:
    xor eax, eax
    pop rsi
    pop rdi
    ret

.global stub_WriteFile
stub_WriteFile:
    push rdi
    push rsi
    mov r10, r9
    movsxd rdi, ecx
    test rdi, rdi
    jns 1f
    neg rdi
    sub rdi, 10
1:
    mov rsi, rdx
    mov rdx, r8
    mov eax, 1
    syscall
    test rax, rax
    js 2f
    test r10, r10
    jz 3f
    mov dword ptr [r10], eax
3:
    mov eax, 1
    pop rsi
    pop rdi
    ret
2:
    test r10, r10
    jz 4f
    mov dword ptr [r10], 0
4:
    xor eax, eax
    pop rsi
    pop rdi
    ret

.global stub_CreateFileA
stub_CreateFileA:
    push rdi
    push rsi
    mov rdi, rcx
    xor esi, esi
    xor edx, edx
    mov eax, 2
    syscall
    test rax, rax
    js 1f
    neg rax
    sub rax, 10
    pop rsi
    pop rdi
    ret
1:
    mov rax, -1
    pop rsi
    pop rdi
    ret

.global stub_CloseHandle
stub_CloseHandle:
    push rdi
    movsxd rdi, ecx
    test rdi, rdi
    jns 1f
    neg rdi
    sub rdi, 10
1:
    mov eax, 3
    syscall
    test rax, rax
    sete al
    movzx eax, al
    pop rdi
    ret

.global stub_GetProcessHeap
stub_GetProcessHeap:
    xor eax, eax
    ret

.global stub_GetLocalTime
stub_GetLocalTime:
    mov qword ptr [rcx], 0
    mov qword ptr [rcx + 8], 0
    ret

.global stub_HeapAlloc
stub_HeapAlloc:
    push rdi
    push rsi
    mov rsi, r8
    add rsi, 31
    and rsi, -16
    add rsi, 16
    xor edi, edi
    mov edx, 3
    mov r10, 34
    mov r8, -1
    xor r9d, r9d
    mov eax, 9
    syscall
    test rax, rax
    js 1f
    mov [rax], rsi
    add rax, 16
    pop rsi
    pop rdi
    ret
1:
    xor eax, eax
    pop rsi
    pop rdi
    ret

.global stub_HeapFree
stub_HeapFree:
    test r8, r8
    jz 2f
    push rdi
    push rsi
    mov rdi, r8
    sub rdi, 16
    mov rsi, [rdi]
    mov eax, 11
    syscall
    pop rsi
    pop rdi
    test rax, rax
    sete al
    movzx eax, al
    ret
2:
    mov eax, 1
    ret

.global stub_GetEnvironmentVariableA
stub_GetEnvironmentVariableA:
    push rsi
    push rdi
    lea rsi, [rip + .Lname_include]
    xor r9d, r9d
1:
    mov al, [rcx + r9]
    cmp al, [rsi + r9]
    jne 2f
    test al, al
    jz .Linclude
    inc r9
    jmp 1b
2:
    lea rsi, [rip + .Lname_standard]
    xor r9d, r9d
3:
    mov al, [rcx + r9]
    cmp al, [rsi + r9]
    jne .Lnot_found
    test al, al
    jz .Lstandard
    inc r9
    jmp 3b
.Linclude:
    lea rdi, [rip + .Lvalue_include]
    jmp .Lcopy
.Lstandard:
    lea rdi, [rip + .Lvalue_standard]
.Lcopy:
    xor r9d, r9d
4:
    mov al, [rdi + r9]
    test al, al
    jz 5f
    inc r9
    jmp 4b
5:
    lea rax, [r9 + 1]
    cmp r8, rax
    jb .Lsmall
    xor r10d, r10d
6:
    mov al, [rdi + r10]
    mov [rdx + r10], al
    inc r10
    cmp r10, rax
    jb 6b
    mov rax, r9
    jmp .Ldone
.Lsmall:
    lea rax, [r9 + 1]
    jmp .Ldone
.Lnot_found:
    xor eax, eax
.Ldone:
    pop rdi
    pop rsi
    ret

.Lname_include:
    .asciz "C1_INCLUDE"
.Lname_standard:
    .asciz "C1_STANDARD"
.Lvalue_include:
    .asciz "include"
.Lvalue_standard:
    .asciz "c99"
