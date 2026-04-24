;
; File: isr_asm.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 19 Mar 2026
; Modified By: BjornBEs
; -----
;

[bits 64]

extern x86_ISR_handler

; cpu pushes to the stack: ss, esp, eflags, cs, eip

%macro ISR_NOERRORCODE 1

global x86_ISR%1:
x86_ISR%1:
    push qword 0              ; push dummy error code
    push qword %1             ; push interrupt number
    jmp isr_common

%endmacro

%macro ISR_ERRORCODE 1
global x86_ISR%1:
x86_ISR%1:
                        ; cpu pushes an error code to the stack
    push qword %1             ; push interrupt number
    jmp isr_common

%endmacro

%include "arch/x86/isrs_gen.inc"

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
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rdi, rsp ; pass pointer to stack to C
    call x86_ISR_handler

    pop rax             ; restore old segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    
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

isr_common_return:
    ;
    ; push ss, esp, eflags, cs, eip
    ;
    ; push rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi, r8-15
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
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rdi, rsp ; pass pointer to stack to C
    call x86_ISR_handler

    pop rdx             ; restore old segment
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx

    
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
    add rsp, 16              ; remove error code and interrupt number
    iretq