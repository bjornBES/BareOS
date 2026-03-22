;
; File: gdt_asm.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 19 Mar 2026
; Modified By: BjornBEs
; -----
;

[bits 32]

; void __attribute__((cdecl)) GDT_Load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);
global GDT_Load
GDT_Load:
    ; make new call frame
    push ebp             ; save old call frame
    mov ebp, esp         ; initialize new call frame
    cli
    
    ; load gdt
    mov eax, [ebp + 8]
    lgdt [eax]

    ; reload code segment
    jmp 0x08:.reload_cs

.reload_cs:
    mov al, 'H'
    out 0xe9, al
    
    ; reload data segments
    mov eax, [ebp + 16]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    sti
    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret

; void __attribute__((cdecl)) i686_tss_load();
global i686_tss_load
i686_tss_load:
    mov ax, (5 * 8) | 0 ; fifth 8-byte selector, symbolically OR-ed with 0 to set the RPL (requested privilege level).
    ltr ax
	
    ret