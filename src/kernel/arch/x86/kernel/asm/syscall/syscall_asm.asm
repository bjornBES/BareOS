;
; File: syscall_asm.asm
; File Created: 06 Jul 2026
; Author: BjornBEs
; -----
; Last Modified: 07 Jul 2026
; Modified By: BjornBEs
; -----
;

CPU_USER_RSP    equ     8
CPU_KERNEL_RSP  equ     16
CURRENT         equ     24

KERNEL_STACK    equ     8

FRAME_QWORDS    equ     18						; user_rsp, r11, rcx, rax..r15

extern syscall_dispatch

global syscall_entry
syscall_entry:
	swapgs

	mov     [gs:CPU_USER_RSP], rsp
	mov     rsp, [gs:CPU_KERNEL_RSP]

	; int 	1

	push    qword [gs:CPU_USER_RSP]

	push    r11

	push    rcx

	push    rax
	push    rcx
	push    rdx
	push    rbx
	push    rbp
	push    rsi
	push    rdi
	push    r8
	push    r9
	push    r10
	push    r11
	push    r12
	push    r13
	push    r14
	push    r15
	
	mov     rax, [gs:CURRENT]
	mov     rbx, [rax + KERNEL_STACK]			; top of this thread's own kernel stack
	sub     rbx, FRAME_QWORDS*8					; reserve space for the frame there

	mov     rsi, rsp							; src = per-cpu stack (old frame)
	mov     rdi, rbx							; dst = thread's stack (new frame location)
	mov     rcx, FRAME_QWORDS
	cld
	rep     movsq								; copy the 18 qwords over

	mov     rsp, rbx							; NOW switch rsp — frame is already here
	mov     rdi, rsp							; arg to syscall_dispatch = new frame ptr
	call    syscall_dispatch
	
	; int 	1
	
	pop     r15
	pop     r14
	pop     r13
	pop     r12
	pop     r11
	pop     r10
	pop     r9
	pop     r8
	pop     rdi
	pop     rsi
	pop     rbp
	pop     rbx
	pop     rdx
	pop     rcx
	pop     rax
	; int 	1
global .pop_rest
.pop_rest:
	pop     rcx
	pop     r11
	pop     rsp
	; int 	1

	swapgs
	o64     sysret
