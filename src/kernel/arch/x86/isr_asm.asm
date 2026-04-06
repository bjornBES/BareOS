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
    push dword 0              ; push dummy error code
    push dword %1             ; push interrupt number
    jmp isr_common

%endmacro

%macro ISR_ERRORCODE 1
global x86_ISR%1:
x86_ISR%1:
                        ; cpu pushes an error code to the stack
    push dword %1             ; push interrupt number
    jmp isr_common

%endmacro

%include "arch/x86/isrs_gen.inc"

isr_common:
    ;
    ; push ss, esp, eflags, cs, eip
    ;
    ; push rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi
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
    
    xor rax, rax        ; push ds
    mov ax, ds
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

    
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rbp
    push rbx
    push rdx
    push rcx
    push rax
    add rsp, 16              ; remove error code and interrupt number
    iret