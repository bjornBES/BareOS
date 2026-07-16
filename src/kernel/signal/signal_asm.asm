;
; File: signal_asm.asm
; File Created: 01 May 2026
; Author: BjornBEs
; -----
; Last Modified: 01 May 2026
; Modified By: BjornBEs
; -----
;

section .vdso
[bits 64]
global __signal_trampoline
__signal_trampoline:
    mov rax, 15
    int 0x80