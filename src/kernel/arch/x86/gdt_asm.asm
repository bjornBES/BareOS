;
; File: gdt_asm.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 19 Mar 2026
; Modified By: BjornBEs
; -----
;
%if i686 == 1
[bits 32]
; void __attribute__((cdecl)) GDT_load_32(GDTDescriptor* descriptor);
global GDT_load_32
GDT_load_32:
   
    ; load gdt
    lgdt [esp+4]

    ; reload code segment
    jmp 0x08:.reload_cs

.reload_cs:
    
    ; reload data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    sti

    ret
%endif
[bits 64]
; void GDT_load_64(GDTDescriptor* descriptor);
global GDT_load_64
GDT_load_64:
   
    ; load gdt
    lgdt [rdi]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; reload CS via a far return
    pop rdi             ; return address
    push 0x08           ; kernel code segment selector
    push rdi
    retfq               ; far return, loads CS:RIP