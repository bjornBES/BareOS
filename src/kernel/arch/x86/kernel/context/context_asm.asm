;
; File: context_asm.asm
; File Created: 22 Sep 2026
; Author: BjornBEs
; -----
; Last Modified: 22 Sep 2026
; Modified By: BjornBEs
; -----
;


extern isr_restore

; void context_arch_switch(vaddr_t new)
global context_arch_switch
context_arch_switch:
    mov rsp, rdi
    jmp isr_restore