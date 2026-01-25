[bits 32]

extern i686_ISRHandler

; cpu pushes to the stack: ss, esp, eflags, cs, eip

%macro ISR_NOERRORCODE 1

global i686_ISR%1:
i686_ISR%1:
    push 0              ; push dummy error code
    push %1             ; push interrupt number
    jmp isr_common

%endmacro

%macro ISR_ERRORCODE 1
global i686_ISR%1:
i686_ISR%1:
                        ; cpu pushes an error code to the stack
    push %1             ; push interrupt number
    jmp isr_common

%endmacro

%include "arch/i686/isrs_gen.inc"

isr_common:
    ;
    ; push ss, esp, eflags, cs, eip
    ;
    pusha
    ; push eax, ecx, edx, ebx, esp, ebp, esi, edi

    push ds
    push es
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp ; pass pointer to stack to C
    call i686_ISRHandler
    add esp, 4

    pop ax
    pop ds
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa
    add esp, 8              ; remove error code and interrupt number
    iret