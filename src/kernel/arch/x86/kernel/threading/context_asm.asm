;
; File: context_asm.asm
; File Created: 02 May 2026
; Author: BjornBEs
; -----
; Last Modified: 02 May 2026
; Modified By: BjornBEs
; -----
;

extern isr_restore

; void context_switch(reg_t new)
global context_switch
context_switch:
    mov rsp, rdi
    jmp isr_restore