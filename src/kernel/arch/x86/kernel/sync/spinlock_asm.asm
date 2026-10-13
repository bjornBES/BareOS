;
; File: spinlock_asm.asm
; File Created: 18 Sep 2026
; Author: BjornBEs
; -----
; Last Modified: 18 Sep 2026
; Modified By: BjornBEs
; -----
;



[bits 64]

;
; void spinlock_arch_acquire(spinlock_t *lock);
;
global spinlock_arch_acquire
spinlock_arch_acquire:
    push rax
    push rbx
    pushfq
    cli
    mov eax, 1
    ; mov ebx, 1
.retry:
    ; Compare EAX with [rdi]. If equal, ZF is set and ebx is loaded into [rdi]. Else, clear ZF and load [rdi] into EAX.
    xchg eax, dword [rdi]
    test eax, eax
    jz .done
.spin:
    pause
    cmp dword [rdi], 0
    jnz .spin
    mov eax, 1
    jmp .retry
.done:
    pop [rdi + 4]
    pop rbx
    pop rax
    ret

;
; void spinlock_arch_release(spinlock_t *lock);
;
global spinlock_arch_release
spinlock_arch_release:
    push qword [rdi + 4]
    push rax
    xor rax, rax
    mov qword [rdi + 4], rax
    mov dword [rdi], eax
    pop rax
    popfq
    ret