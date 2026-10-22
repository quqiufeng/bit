.intel_syntax noprefix
.global _start

.section .text
_start:
    lea r13, [rip + inbuf]
    lea r14, [rip + outbuf]
    lea r15, [rip + labels]

    xor r12, r12
.read_loop:
    mov rax, 0
    mov rdi, 0
    lea rsi, [r13 + r12]
    mov rdx, 1048576
    sub rdx, r12
    syscall
    test rax, rax
    jle .read_done
    add r12, rax
    cmp r12, 1048576
    jb .read_loop
.read_done:
    mov [rip + in_len], r12

    xor r12, r12
    call run_pass
    mov rax, [rip + max_offset]
    mov [rip + out_len], rax
    mov r12, 1
    call run_pass

    mov rax, 1
    mov rdi, 1
    lea rsi, [rip + outbuf]
    mov rdx, [rip + out_len]
    syscall

    mov rax, 60
    xor rdi, rdi
    syscall

run_pass:
    xor ebx, ebx
    mov qword ptr [rip + base_vaddr], 0
    mov qword ptr [rip + cur_vaddr], 0
    mov qword ptr [rip + cur_offset], 0
    mov qword ptr [rip + max_offset], 0
.run_loop:
    call skip_space
    cmp rbx, [rip + in_len]
    jae .run_done
    mov al, [r13 + rbx]
    cmp al, '^'
    je .run_origin
    cmp al, ':'
    je .run_label
    cmp al, '%'
    je .run_rel

    movzx eax, byte ptr [r13 + rbx]
    call hex_val
    cmp eax, -1
    je fail
    mov r8d, eax
    lea rdx, [rbx + 1]
    movzx eax, byte ptr [r13 + rdx]
    call hex_val
    cmp eax, -1
    je fail
    shl r8d, 4
    or r8d, eax
    mov eax, r8d
    call emit_byte
    add rbx, 2
    mov rdi, 1
    call advance
    jmp .run_loop

.run_origin:
    inc rbx
    call count_hex
    test ecx, ecx
    jz fail
    call read_hex_n
    mov [rip + base_vaddr], rax
    mov rdx, [rip + cur_offset]
    add rax, rdx
    mov [rip + cur_vaddr], rax
    jmp .run_loop

.run_label:
    inc rbx
    mov ecx, 4
    call read_hex_n
    cmp rax, 65536
    jae fail
    mov [rip + cur_offset], rax
    mov rdx, [rip + base_vaddr]
    add rdx, rax
    mov [rip + cur_vaddr], rdx
    imul rcx, rax, 24
    lea rdi, [r15 + rcx]
    mov qword ptr [rdi], 1
    mov [rdi + 8], rdx
    mov [rdi + 16], rax
    mov rdx, [rip + max_offset]
    cmp rdx, rax
    jae .run_loop
    mov [rip + max_offset], rax
    jmp .run_loop

.run_rel:
    inc rbx
    mov ecx, 4
    call read_hex_n
    cmp rax, 65536
    jae fail
    imul rcx, rax, 24
    lea rdi, [r15 + rcx]
    cmp qword ptr [rdi], 0
    je fail
    mov rdx, [rdi + 8]
    mov rcx, [rip + cur_vaddr]
    add rcx, 4
    sub rdx, rcx
    mov eax, edx
    call emit_dword
    mov rdi, 4
    call advance
    jmp .run_loop
.run_done:
    ret

skip_space:
.skip_loop:
    cmp rbx, [rip + in_len]
    jae .skip_done
    mov al, [r13 + rbx]
    cmp al, ';'
    je .skip_comment
    cmp al, ' '
    je .skip_advance
    cmp al, 9
    je .skip_advance
    cmp al, 10
    je .skip_advance
    cmp al, 13
    je .skip_advance
    ret
.skip_advance:
    inc rbx
    jmp .skip_loop
.skip_comment:
    inc rbx
    cmp rbx, [rip + in_len]
    jae .skip_done
    cmp byte ptr [r13 + rbx], 10
    jne .skip_comment
    jmp .skip_loop
.skip_done:
    ret

hex_val:
    movzx eax, al
    cmp eax, 48
    jb .bad
    cmp eax, 57
    jbe .digit
    cmp eax, 65
    jb .bad
    cmp eax, 70
    jbe .upper
    cmp eax, 97
    jb .bad
    cmp eax, 102
    ja .bad
    sub eax, 87
    ret
.upper:
    sub eax, 55
    ret
.digit:
    sub eax, 48
    ret
.bad:
    mov eax, -1
    ret

count_hex:
    xor ecx, ecx
.count_loop:
    lea rdx, [rbx + rcx]
    cmp rdx, [rip + in_len]
    jae .count_done
    mov al, [r13 + rdx]
    call hex_val
    cmp eax, -1
    je .count_done
    inc ecx
    jmp .count_loop
.count_done:
    ret

read_hex_n:
    mov r8d, ecx
    xor r9d, r9d
.rh_loop:
    mov al, [r13 + rbx]
    call hex_val
    cmp eax, -1
    je fail
    shl r9, 4
    or r9, rax
    inc rbx
    dec r8d
    jnz .rh_loop
    mov rax, r9
    ret

advance:
    mov rax, [rip + cur_offset]
    add rax, rdi
    mov [rip + cur_offset], rax
    mov rax, [rip + cur_vaddr]
    add rax, rdi
    mov [rip + cur_vaddr], rax
    mov rax, [rip + max_offset]
    cmp rax, [rip + cur_offset]
    jae .adv_done
    mov rax, [rip + cur_offset]
    mov [rip + max_offset], rax
.adv_done:
    ret

emit_byte:
    test r12, r12
    jz .eb_done
    mov rdx, [rip + cur_offset]
    cmp rdx, 1048576
    jae fail
    mov [r14 + rdx], al
.eb_done:
    ret

emit_dword:
    test r12, r12
    jz .ed_done
    mov rdx, [rip + cur_offset]
    cmp rdx, 1048572
    ja fail
    mov [r14 + rdx], eax
.ed_done:
    ret

fail:
    mov rax, 1
    mov rdi, 2
    lea rsi, [rip + error_message]
    mov rdx, 9
    syscall
    mov rax, 60
    mov rdi, 1
    syscall

.section .rodata
error_message:
    .ascii "a0 error\n"

.section .bss
.align 16
in_len:
    .skip 8
base_vaddr:
    .skip 8
cur_vaddr:
    .skip 8
cur_offset:
    .skip 8
max_offset:
    .skip 8
out_len:
    .skip 8
.align 16
inbuf:
    .skip 1048576
.align 16
outbuf:
    .skip 1048576
.align 16
labels:
    .skip 1572864
