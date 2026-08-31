;
; File: isr_asm.asm
; File Created: 31 Aug 2026
; Author: BjornBEs
; -----
; Last Modified: 31 Aug 2026
; Modified By: BjornBEs
; -----
;

[bits 64]

extern isr_handler

%macro ISR_NOERRORCODE 1

global x86_isr_%1:
x86_isr_%1:
    push qword 0              ; push dummy error code
    push qword %1             ; push interrupt number
    jmp isr_common

%endmacro

%macro ISR_ERRORCODE 1
global x86_isr_%1:
x86_isr_%1:
                        ; cpu pushes an error code to the stack
    push qword %1             ; push interrupt number
    jmp isr_common

%endmacro

%include "kernel/isr/isrs_gen.inc"
%include "config.inc"

%if __x86_64__
isr_common:
    ;
    ; push ss, esp, eflags, cs, eip
    ;
    ; push rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi, r8-15
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    mov rax, ds
    push rax
    mov rax, es
    push rax
    
%if CONFIG_ENABLE_SMP==0
    mov rax, fs
    push rax
    mov rax, gs
    push rax
%else
    xor rax, rax
    push rax
    push rax
%endif

    mov ax, 0x10
    mov ds, ax
    mov es, ax
%if CONFIG_ENABLE_SMP==0
    mov fs, ax
    mov gs, ax
%endif
    mov rdi, rsp ; pass pointer to stack to C

    call isr_handler
global isr_restore
isr_restore:
    
    pop rax             ; restore old segment
    mov ds, ax
    mov es, ax
%if CONFIG_ENABLE_SMP==0
    mov fs, ax
    mov gs, ax
%endif

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
    add rsp, 16              ; remove error code and interrupt number
    iretq
%endif