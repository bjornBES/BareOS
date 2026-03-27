;
; File: process_asm.asm
; File Created: 19 Mar 2026
; Author: BjornBEs
; -----
; Last Modified: 19 Mar 2026
; Modified By: BjornBEs
; -----
;

;
; void ASMCALL jump_to_user(void *user_entry, void *user_stack, pid process)
;
global jump_to_user
jump_to_user:
    
    mov         ax,             0x23            ; user data
    mov         ds,             ax
    mov         es,             ax
    mov         fs,             ax
    mov         gs,             ax
    
    mov         rbx,            rdi             ; process
    
    push        0x23                            ; user data
    push        rsi
    
    pushf
    pop         rdx
    or          rdx,            0x200
    push        rdx
    push        0x1B                            ; user code
    push        rdi

    iret