;
; File: main.asm
; File Created: 15 Mar 2026
; Author: BjornBEs
; -----
; Last Modified: 15 Mar 2026
; Modified By: BjornBEs
; -----
;
[bits 64]
section .text 
global main
main:

    mov         rdi,            1
    mov         rax,            512
    int         0x80
    
    mov         rdi,            3
    mov         rsi,            MESSAGE
    mov         rdx,            [MESSAGE_COUNT]
    mov         rax,            1
    int         0x80
    mov         rdi,            1
    mov         rsi,            MESSAGE
    mov         rdx,            [MESSAGE_COUNT]
    mov         rax,            1
    int         0x80
    jmp         $
    ret

section .data
    MESSAGE: db "hello world from user space", 0xA
    MESSAGE_COUNT: dd $-MESSAGE