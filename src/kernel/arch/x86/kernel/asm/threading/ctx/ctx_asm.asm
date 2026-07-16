;
; File: ctx_asm.asm
; File Created: 02 May 2026
; Author: BjornBEs
; -----
; Last Modified: 01 Jun 2026
; Modified By: BjornBEs
; -----
;

extern isr_restore

; void ctx_arch_switch(vaddr_t new)
global ctx_arch_switch
ctx_arch_switch:
    mov rsp, rdi
    jmp isr_restore