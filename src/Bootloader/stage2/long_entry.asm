;
; File: long_entry.asm
; File Created: 23 Mar 2026
; Author: BjornBEs
; -----
; Last Modified: 23 Mar 2026
; Modified By: BjornBEs
; -----
;

extern kernelEntry
; yes i don't know what i am doing with my life
global entry_but_long
entry_but_long:
    lgdt    [GDTDesc_64]

    ; fuck you 32 bit
    jmp     0x08:long_mode_entry
    
[bits 64]
long_mode_entry:
    mov     ax, 0x10            ; data selector (offset 0x10)
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    mov     rdi, 0x00200000     ; bootparams
    mov     rax, [kernelEntry]
    jmp     rax

GDT_64:
    ; Null Descriptor
    dq 0

    ; 64-bit code segment
    dw 0xFFFF       ; limit (bits 0-15)
    dw 0            ; base (bits 0-15)
    db 0            ; base (bits 16-23)
    db 10011010b    ; access
    db 00100000b ; limit (bits 16-19) + flags
    db 0            ; base (bits 24-31)
    
    ; 64-bit data segment
    dw 0xFFFF       ; limit (bits 0-15)
    dw 0            ; base (bits 0-15)
    db 0            ; base (bits 16-23)
    db 10010010b    ; access
    db 00100000b ; limit (bits 16-19) + flags
    db 0            ; base (bits 24-31)

GDTDesc_64: dw GDTDesc_64 - GDT_64 - 1
            dq GDT_64