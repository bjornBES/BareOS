;
; File: spinlock_asm.asm
; File Created: 04 Aug 2026
; Author: BjornBEs
; -----
; Last Modified: 04 Aug 2026
; Modified By: BjornBEs
; -----
;

[bits 64]

;
; void spinlock_arch_lock(uint32_t *lock);
;
global spinlock_arch_lock
spinlock_arch_lock:
    mov eax, 1
.retry:
    xchg eax, [rdi]
    test eax, eax
    jz .done
.spin:
    pause
    cmp dword [rdi], 0
    jnz .spin
    mov eax, 1
    jmp .retry
.done:
    ret

;
; void spinlock_arch_unlock(uint32_t *lock);
;
global spinlock_arch_unlock
spinlock_arch_unlock:
    mov dword [rdi], 0
    ret