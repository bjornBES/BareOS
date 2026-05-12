;
; File: idt_asm.asm
; File Created: 07 Apr 2026
; Author: BjornBEs
; -----
; Last Modified: 07 Apr 2026
; Modified By: BjornBEs
; -----
;

%if __i686__ == 1
[bits 32]
; void __attribute__((cdecl)) idt_load_32(GDTDescriptor* descriptor);
global idt_load_32
idt_load_32:
   
    ; load gdt
    lidt [esp+4]
    sti
    ret
%endif
[bits 64]
; void idt_load_64(GDTDescriptor* descriptor);
global idt_load_64
idt_load_64:
   
    ; load gdt
    lidt [rdi]
    sti
    ret