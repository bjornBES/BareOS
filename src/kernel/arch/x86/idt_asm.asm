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
; void __attribute__((cdecl)) IDT_Load_32(GDTDescriptor* descriptor);
global IDT_Load_32
IDT_Load_32:
   
    ; load gdt
    lidt [esp+4]
    sti
    ret
%endif
[bits 64]
; void IDT_Load_64(GDTDescriptor* descriptor);
global IDT_Load_64
IDT_Load_64:
   
    ; load gdt
    lidt [rdi]
    sti
    ret