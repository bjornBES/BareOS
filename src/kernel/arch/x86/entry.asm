;
; File: entry.asm
; File Created: 23 Aug 2026
; Author: BjornBEs
; -----
; Last Modified: 23 Aug 2026
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
%endif
%if __i686__=1
    [bits 32]
%endif
    cli
    
    mov al, 'H'
    out 0xe9, al

%if __x86_64__=1
    mov rsp, stack_top
    mov rbp, rsp
    push rdi
%endif
%if __i686__=1
    mov esp, stack_top
    mov esp, esp
    push edi
%endif

    mov al, 'H'
    out 0xe9, al
    call kernel_main

.end:
    jmp .end