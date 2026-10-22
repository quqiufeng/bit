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
    mov r10, [rsp + 40]
    mov r11, [rsp + 48]
    push rdi
    push rsi
    mov rdi, rcx
    xor esi, esi
    test rdx, 0x40000000
    jz .Lcf_read
    mov esi, 1
.Lcf_read:
    cmp r10, 1
    jne .Lcf_not_new
    or esi, 0xc0
    jmp .Lcf_open
.Lcf_not_new:
    cmp r10, 2
    jne .Lcf_not_always
    or esi, 0x240
    jmp .Lcf_open
.Lcf_not_always:
    cmp r10, 4
    jne .Lcf_open
    or esi, 0x40
.Lcf_open:
    mov edx, 438
    mov eax, 2
    syscall
    test rax, rax
    js .Lcf_fail
    neg rax
    sub rax, 10
    pop rsi
    pop rdi
    ret
.Lcf_fail:
    mov rax, -1
    pop rsi
    pop rdi
    ret

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

.global stub_GetCommandLineA
stub_GetCommandLineA:
    mov rax, 0x910000
    mov rdx, [rax]
    test rdx, rdx
    jz .Lcmd_empty
    mov r11, [rdx]
    lea r8, [rdx + 8]
    mov r9, 0x910008
    xor r10d, r10d
.Lcmd_arg:
    cmp r10, r11
    jae .Lcmd_done
    test r10, r10
    jz .Lcmd_copy
    mov byte ptr [r9], 32
    inc r9
.Lcmd_copy:
    mov rdx, [r8 + r10 * 8]
.Lcmd_char:
    mov al, [rdx]
    test al, al
    jz .Lcmd_next
    mov [r9], al
    inc r9
    inc rdx
    jmp .Lcmd_char
.Lcmd_next:
    inc r10
    jmp .Lcmd_arg
.Lcmd_done:
    mov byte ptr [r9], 0
    mov rax, 0x910008
    ret
.Lcmd_empty:
    mov byte ptr [0x910008], 0
    mov rax, 0x910008
    ret

.global stub_GetLastError
stub_GetLastError:
    xor eax, eax
    ret

.global stub_GetFileType
stub_GetFileType:
    mov eax, 1
    ret

.global stub_FlushFileBuffers
stub_FlushFileBuffers:
    mov eax, 1
    ret


.global stub_GetConsoleMode
stub_GetConsoleMode:
    xor eax, eax
    ret

.global stub_GetSystemTimeAsFileTime
stub_GetSystemTimeAsFileTime:
    mov qword ptr [rcx], 0
    ret

.global stub_GetTimeZoneInformation
stub_GetTimeZoneInformation:
    xor eax, eax
    ret

.global stub_GetTickCount64
stub_GetTickCount64:
    xor eax, eax
    ret

.global stub_GetCurrentProcessId
stub_GetCurrentProcessId:
    mov eax, 1
    ret

.global stub_GetTempPathA
stub_GetTempPathA:
    mov byte ptr [rdx], 46
    mov byte ptr [rdx + 1], 0
    mov eax, 1
    ret

.global stub_GetTempFileNameA
stub_GetTempFileNameA:
    xor eax, eax
    ret

.global stub_GetProcessTimes
stub_GetProcessTimes:
    mov eax, 1
    ret

.global stub_GetCurrentProcess
stub_GetCurrentProcess:
    mov rax, -1
    ret

.global stub_GetDynamicTimeZoneInformation
stub_GetDynamicTimeZoneInformation:
    xor eax, eax
    ret

.global stub_GetTimeZoneInformationForYear
stub_GetTimeZoneInformationForYear:
    xor eax, eax
    ret

.global stub_RemoveDirectoryA
stub_RemoveDirectoryA:
    xor eax, eax
    ret

.global stub_GetFileAttributesA
stub_GetFileAttributesA:
    mov rax, -1
    ret

.global stub_GetSystemDirectoryA
stub_GetSystemDirectoryA:
    xor eax, eax
    ret

.global stub_CreateProcessA
stub_CreateProcessA:
    xor eax, eax
    ret

.global stub_WaitForSingleObject
stub_WaitForSingleObject:
    xor eax, eax
    ret

.global stub_GetExitCodeProcess
stub_GetExitCodeProcess:
    xor eax, eax
    ret


.global stub_SetFilePointerEx
stub_SetFilePointerEx:
    push rdi
    push rsi
    movsxd rdi, ecx
    test rdi, rdi
    jns .Lsfp_fd
    neg rdi
    sub rdi, 10
.Lsfp_fd:
    mov rsi, rdx
    mov rdx, r9
    mov eax, 8
    syscall
    test rax, rax
    js .Lsfp_fail
    test r8, r8
    jz .Lsfp_ok
    mov [r8], rax
.Lsfp_ok:
    mov eax, 1
    pop rsi
    pop rdi
    ret
.Lsfp_fail:
    xor eax, eax
    pop rsi
    pop rdi
    ret

.global stub_GetFileSizeEx
stub_GetFileSizeEx:
    push rdi
    push rsi
    push rbx
    push r12
    movsxd rdi, ecx
    test rdi, rdi
    jns .Lgfs_fd
    neg rdi
    sub rdi, 10
.Lgfs_fd:
    mov rbx, rdx
    xor esi, esi
    mov edx, 1
    mov eax, 8
    syscall
    test rax, rax
    js .Lgfs_fail
    mov r12, rax
    xor esi, esi
    mov edx, 2
    mov eax, 8
    syscall
    test rax, rax
    js .Lgfs_fail
    mov [rbx], rax
    mov rsi, r12
    xor edx, edx
    mov eax, 8
    syscall
    mov eax, 1
    pop r12
    pop rbx
    pop rsi
    pop rdi
    ret
.Lgfs_fail:
    xor eax, eax
    pop r12
    pop rbx
    pop rsi
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
.Lcmp_include:
    mov al, [rcx + r9]
    cmp al, [rsi + r9]
    jne .Lcmp_standard
    test al, al
    jz .Linclude
    inc r9
    jmp .Lcmp_include
.Lcmp_standard:
    lea rsi, [rip + .Lname_standard]
    xor r9d, r9d
.Lcmp_standard_loop:
    mov al, [rcx + r9]
    cmp al, [rsi + r9]
    jne .Lcmp_output
    test al, al
    jz .Lstandard
    inc r9
    jmp .Lcmp_standard_loop
.Lcmp_output:
    lea rsi, [rip + .Lname_output]
    xor r9d, r9d
.Lcmp_output_loop:
    mov al, [rcx + r9]
    cmp al, [rsi + r9]
    jne .Lnot_found
    test al, al
    jz .Loutput
    inc r9
    jmp .Lcmp_output_loop
.Linclude:
    lea rdi, [rip + .Lvalue_include]
    jmp .Lcopy
.Lstandard:
    lea rdi, [rip + .Lvalue_standard]
    jmp .Lcopy
.Loutput:
    cmp byte ptr [rip + data_c1_output], 0
    je .Lnot_found
    lea rdi, [rip + data_c1_output]
.Lcopy:
    xor r9d, r9d
.Lcopy_loop:
    mov al, [rdi + r9]
    test al, al
    jz .Lcopy_end
    inc r9
    jmp .Lcopy_loop
.Lcopy_end:
    lea rax, [r9 + 1]
    cmp r8, rax
    jb .Lsmall
    xor r10d, r10d
.Lcopy_bytes:
    mov al, [rdi + r10]
    mov [rdx + r10], al
    inc r10
    cmp r10, rax
    jb .Lcopy_bytes
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
.Lname_output:
    .asciz "C1_OUTPUT"
data_c1_output:
    .skip 8
