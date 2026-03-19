;
; File: compiler.asm
; File Created: 18 Mar 2026
; Author: BjornBEs
; -----
; Last Modified: 18 Mar 2026
; Modified By: BjornBEs
; -----
;

[bits 16]

section .entry
global start
start:
    xor     ax, ax
    mov     ds, ax
    mov     es, ax
    
    mov     ss, ax
    mov     sp, 0x7000

    jmp     0:entry
entry:
    

section .text

section .data 

program:
db ""

section .bss 


