;
; File: long_entry.asm
; File Created: 23 Mar 2026
; Author: BjornBEs
; -----
; Last Modified: 23 Mar 2026
; Modified By: BjornBEs
; -----
;

[bits 32]
extern kernelEntry
extern fill_64bit_table
extern _cr3
extern bss_bootParams
; yes i don't know what i am doing with my life
global entry_but_long
entry_but_long:
    %if __x86_64__=1

; Enabling Physical Address Extension and Page Size Extension 
    mov     eax,        cr4
    or      eax,        (1 << 5) | (1 << 4)
    and     eax,        ~(1 << 31)
    mov     cr4,        eax

    call    fill_64bit_table

    mov     eax,        [_cr3]
    mov     cr3,        eax

    mov al, 'A'
    out 0xe9, al

    mov     ecx, 0xC0000080
    rdmsr
    or      eax, 1 << 8
    wrmsr

    ; fuck yes 64 bits fuck yes
    mov al, 'E'
    out 0xe9, al
    mov     eax, cr0
    or      eax, 1 << 31
    mov     cr0, eax
    
    global .end
.end:
    ; fuck you 32 bit
    jmp     0x28:long_mode_entry
            
[bits 64]
global long_mode_entry
long_mode_entry:

    mov al, 'R'
    out 0xe9, al
    
    mov     ax, 0x30            ; data selector (offset 0x30)
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    
    mov     rdi,    GDTDesc_64
    lgdt    [rdi]

    mov     rdi,    .reload
    push    0x8
    push    rdi
    retfq
global .reload
.reload:
    mov     ax, 0x10            ; data selector (offset 0x30)
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    
    lea     rdi, [0x600]     ; bootparams
    mov     rax, [kernel_entry]
    jmp     rax

GDT_64:
    ; Null Descriptor
    dq 0

    ; 64-bit code segment
    dw 0xFFFF       ; limit (bits 0-15)
    dw 0            ; base (bits 0-15)
    db 0            ; base (bits 16-23)
    db 10011010b    ; access
    db 00101111b    ; limit (bits 16-19) + flags
    db 0            ; base (bits 24-31)
    
    ; 64-bit data segment
    dw 0xFFFF       ; limit (bits 0-15)
    dw 0            ; base (bits 0-15)
    db 0            ; base (bits 16-23)
    db 10010010b    ; access
    db 00101111b    ; limit (bits 16-19) + flags
    db 0            ; base (bits 24-31)

GDTDesc_64: dw GDTDesc_64 - GDT_64 - 1
            dq GDT_64

%endif
section .bss

global kernel_entry
kernel_entry:
    resq 1
