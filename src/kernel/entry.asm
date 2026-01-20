
[bits 32]

extern stack_top
extern main

section .text

global entry
entry:
    mov esp, stack_top
    push edi
    mov edi, 0xb80000
    mov esi, message
    mov ah, 0x07
    mov ecx, message_len

.next_char:
    lodsb
    xchg al, ah
    stosw
    loop .next_char

    call main

.end:
    jmp .end

message:
db "BES KERNEL", 0xD, 0xA, 0
message_len:
    dw $-message