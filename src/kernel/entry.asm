;
; File: entry.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 01 Jul 2026
; Modified By: BjornBEs
; -----
;

extern stack_top
extern kernel_main

section .text

global entry
entry:
%if __x86_64__=1
    [bits 64]
    cli
    
    mov al, 'H'
    out 0xe9, al

    mov rsp, stack_top
    mov rbp, rsp
    push rdi

    mov al, 'H'
    out 0xe9, al
    call kernel_main

.end:
    jmp .end
%endif
%if __i686__=1
    [bits 32]
    cli
    
    mov al, 'H'
    out 0xe9, al

    mov esp, stack_top
    mov esp, esp
    push edi

    mov al, 'H'
    out 0xe9, al
    call kernel_main

.end:
    jmp .end
%endif
