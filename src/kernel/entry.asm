;
; File: entry.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 21 Mar 2026
; Modified By: BjornBEs
; -----
;


[bits 64]

extern stack_top
extern main

section .data

message:
db "BES KERNEL", 0xD, 0xA, 0
message_len:
    dw $-message

section .text

global entry
entry:
    cli
    
    mov al, 'H'
    out 0xe9, al
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, stack_top
    mov rbp, rsp
    push rdi

    mov al, 'H'
    out 0xe9, al,

    ; call main

.end:
    jmp .end

global crash_me
crash_me:
    ; div by 0
    mov rcx, 0x1337
    mov rax, 0
    div rax
    ret