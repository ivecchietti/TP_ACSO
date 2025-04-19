%define NULL 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

extern malloc
extern free
extern str_concat

string_proc_list_create_asm:
    mov rdi, 16
    call malloc
    test rax, rax
    jz .fail
    mov qword [rax], 0
    mov qword [rax + 8], 0
    ret
.fail:
    xor rax, rax
    ret

string_proc_node_create_asm:
    test rsi, rsi
    jz .return_null

    mov r8b, dil       ; guardar type
    mov r9, rsi        ; guardar hash

    mov rdi, 32
    call malloc
    test rax, rax
    jz .return_null

    mov qword [rax], 0
    mov qword [rax + 8], 0
    mov byte [rax + 16], r8b
    mov qword [rax + 24], r9
    ret

.return_null:
    xor rax, rax
    ret

string_proc_list_add_node_asm:
    test rdi, rdi
    jz .null_list

    mov r8, rdi       ; list
    mov r9b, sil      ; type
    mov r10, rdx      ; hash

    movzx rdi, r9b
    mov rsi, r10
    call string_proc_node_create_asm
    test rax, rax
    jz .null_list

    mov r11, rax      ; new node
    mov rcx, [r8 + 8] ; last node

    test rcx, rcx
    jz .empty

    mov [rcx], r11
    mov [r11 + 8], rcx
    mov [r8 + 8], r11
    ret

.empty:
    mov [r8], r11
    mov [r8 + 8], r11
    ret

.null_list:
    xor rax, rax
    ret

string_proc_list_concat_asm:
    test rdi, rdi
    jz .null_ret

    mov r8b, sil       ; type
    mov r9, rdx        ; hash original
    mov r10, rdi       ; list
    mov r11, [r10]     ; nodo actual

    mov rdi, 1
    call malloc
    test rax, rax
    jz .null_ret
    mov byte [rax], 0
    mov r12, rax       ; new_hash

.loop:
    test r11, r11
    jz .done_loop

    mov al, [r11 + 16]
    cmp al, r8b
    jne .next

    mov rdi, r12
    mov rsi, [r11 + 24]
    call str_concat
    mov rdi, r12
    mov r12, rax
    call free

.next:
    mov r11, [r11]
    jmp .loop

.done_loop:
    test r9, r9
    jz .ret_result

    mov rdi, r9
    mov rsi, r12
    call str_concat
    mov rdi, r12
    mov r12, rax
    call free

.ret_result:
    mov rax, r12
    ret

.null_ret:
    xor rax, rax
    ret
