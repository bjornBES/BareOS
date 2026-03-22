;
; File: isr_asm.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 19 Mar 2026
; Modified By: BjornBEs
; -----
;

[bits 32]

extern i686_ISRHandler

; cpu pushes to the stack: ss, esp, eflags, cs, eip

%macro ISR_NOERRORCODE 1

global i686_ISR%1:
i686_ISR%1:
    push dword 0              ; push dummy error code
    push dword %1             ; push interrupt number
    jmp isr_common

%endmacro

%macro ISR_ERRORCODE 1
global i686_ISR%1:
i686_ISR%1:
                        ; cpu pushes an error code to the stack
    push dword %1             ; push interrupt number
    jmp isr_common

%endmacro

%include "arch/i686/isrs_gen.inc"

isr_common:
    ;
    ; push ss, esp, eflags, cs, eip
    ;
    pusha
    ; push eax, ecx, edx, ebx, esp, ebp, esi, edi

    xor eax, eax        ; push ds
    mov ax, ds
    push eax
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp ; pass pointer to stack to C
    call i686_ISRHandler
    add esp, 4

    pop eax             ; restore old segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    
    popa
    add esp, 8              ; remove error code and interrupt number
    iret