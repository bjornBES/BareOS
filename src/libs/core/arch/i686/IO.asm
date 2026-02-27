;
; File: IO.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 27 Feb 2026
; Modified By: BjornBEs
; -----
;

[bits 32]

%if __i686__ == 1

global Outb
Outb:
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global Inb
Inb:
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

global Outw
Outw:
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

global Inw
Inw:
    mov dx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

global Outd
Outd:
    mov dx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

global Ind
Ind:
    mov dx, [esp + 4]
    in eax, dx
    ret

global enableInterrupts
enableInterrupts:
    sti
    ret

global disableInterrupts
disableInterrupts:
    cli
    ret
    
%endif