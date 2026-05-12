;
; File: crt0.asm
; File Created: 15 Mar 2026
; Author: BjornBEs
; -----
; Last Modified: 15 Mar 2026
; Modified By: BjornBEs
; -----
;

[bits 64]
section .text
global _start

extern main

_start:
    xor         rbp,            rbp        ; ABI: mark outermost frame
    
    pop         rdi
    mov         rsi,            rsp
    and         rsp,            ~0xF
        
    ; stack is already set up by the kernel's loader
    call        main
    
    ; main returned — call exit syscall directly
    mov         rdi,            rax             ; exit code from main
    mov         rax,            60              ; syscall number for exit
    int         0x80
    jmp         $