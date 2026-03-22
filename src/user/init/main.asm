;
; File: main.asm
; File Created: 15 Mar 2026
; Author: BjornBEs
; -----
; Last Modified: 15 Mar 2026
; Modified By: BjornBEs
; -----
;

section .text 
global main
main:

    mov         edi,            3
    mov         esi,            MESSAGE
    mov         ecx,            [MESSAGE_COUNT]
    mov         eax,            1
    int         0x80
    jmp         $
    ret

section .data
    MESSAGE: db "hello world from user space", 0xA
    MESSAGE_COUNT: dd $-MESSAGE