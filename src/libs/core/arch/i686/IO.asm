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

global outb
outb:
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global inb
inb:
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

global outw
outw:
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

global inw
inw:
    mov dx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

global outd
outd:
    mov dx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

global ind
ind:
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

global callNMI
callNMI:
    int 0x2
    ret

%endif