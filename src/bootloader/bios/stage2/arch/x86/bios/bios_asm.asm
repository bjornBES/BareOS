;
; File: bios_asm.asm
; File Created: 22 Aug 2026
; Author: BjornBEs
; -----
; Last Modified: 22 Aug 2026
; Modified By: BjornBEs
; -----
;

[bits 32]
section .arch_text


; Convert linear address to segment:offset address
; Args:
;    1 - linear address
;    2 - (out) target segment (e.g. es)
;    3 - target 32-bit register to use (e.g. eax)
;    4 - target lower 16-bit half of #3 (e.g. ax)

%macro LinearToSegOffset 4

    mov %3, %1      ; linear address to eax
    shr %3, 4
    mov %2, %4
    mov %3, %1      ; linear address to eax
    and %3, 0xf

%endmacro

global bios_intcall
bios_intcall:
    push ebp
    mov ebp, esp
    pushfd
    pushad
    push ds
    push es
    push fs
    push gs
    push ss

	; Arguments (cdecl)
    ; [esp+...]
    mov eax, [ebp+8]     ; int_no
    mov ebx, [ebp+12]     ; ireg*
    mov ecx, [ebp+16]     ; oreg*

    mov [.int_number], al
    mov [ireg_ptr], ebx
    mov [oreg_ptr], ecx
    
    [bits 32]
    jmp word 0x18:.pmode16

.pmode16:
    [bits 16]

    mov eax, cr0
    and al, ~1
    mov cr0, eax

    jmp word 00h:.rmode

global .rmode
.rmode:
    mov ax, 0
    mov ds, ax
    mov ss, ax

    sti

    xor ax, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    LinearToSegOffset [bp + 12], ds, esi, si

    ; Load input registers from struct _biosregs
    mov di, [ds:si+0]
    mov bx, [ds:si+4]
    mov dx, [ds:si+6]
    mov cx, [ds:si+8]
    mov ax, [ds:si+10]
    mov gs, [ds:si+12]
    mov fs, [ds:si+14]
    mov es, [ds:si+16]
    
    push ax
    pushf
    pop ax
    or ax, [ds:si+20]
    and ax, ~0x0100
    push ax
    popf
    pop ax

    push ax
    mov ax, [ds:si+18]
    mov si, [ds:si+2]
    mov ds, ax
    pop ax

    ; BIOS call
    db 0xCD
global .int_number
    .int_number:
    db 0


    ; Save output registers
    push di
    push ds

    LinearToSegOffset [bp + 16], ds, edi, di
    
    mov [ds:di+2], si
    mov [ds:di+4], bx
    mov [ds:di+6], dx
    mov [ds:di+8], cx
    mov [ds:di+10], ax
    mov [ds:di+12], gs
    mov [ds:di+14], fs
    mov [ds:di+16], es
    pushf
    pop ax
    mov [ds:di+20], ax
        
    pop ax
    pop bx
    mov [ds:di+18], ax  ; ds    
    mov [ds:di+0], bx   ; di

    cli

    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp 0x08:.pmode
.pmode:
    [bits 32]
	mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Restore PM state
    pop ss
    pop gs
    pop fs
    pop es
    pop ds
    popad
    popfd
    pop ebp
    sti
	ret

ireg_ptr: dd 0
oreg_ptr: dd 0
