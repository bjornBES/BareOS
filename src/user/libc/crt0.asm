;
; File: crt0.asm
; File Created: 15 Mar 2026
; Author: BjornBEs
; -----
; Last Modified: 15 Mar 2026
; Modified By: BjornBEs
; -----
;

section .text
global _start

extern main

_start:
    ; stack is already set up by the kernel's loader
    call        main

    ; main returned — call exit syscall directly
    mov         eax,            60              ; syscall number for exit
    mov         ebx,            0               ; exit code
    int         0x80
    jmp         $