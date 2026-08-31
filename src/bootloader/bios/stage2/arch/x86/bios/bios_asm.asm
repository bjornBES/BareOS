;
; File: bios_asm.asm
; File Created: 22 Aug 2026
; Author: BjornBEs
; -----
; Last Modified: 22 Aug 2026
; Modified By: BjornBEs
; -----
;

	[bits   32]
section .arch_text


; Convert linear address to segment:offset address
; Args:
;    1 - linear address
;    2 - (out) target segment (e.g. es)
;    3 - target 32-bit register to use (e.g. eax)
;    4 - target lower 16-bit half of #3 (e.g. ax)

%macro linear_to_seg_offset 4

	mov     %3, %1								; linear address to eax
	shr     %3, 4
	mov     %2, %4
	mov     %3, %1								; linear address to eax
	and     %3, 0xf

%endmacro

global bios_intcall
bios_intcall:
	push    ebp
	mov     ebp, esp
	pushfd
	pushad
	push    ds
	push    es
	push    fs
	push    gs
	push    ss

	; mov     al, 'S'
	; out     0xE9, al

	; Arguments (cdecl)
	mov     eax, [ebp+8]						; int_no
	mov     ebx, [ebp+12]						; ireg*
	mov     ecx, [ebp+16]						; oreg*

	mov     [.int_number], al
	mov     [ireg_ptr], ebx
	mov     [oreg_ptr], ecx

	[bits   32]
	jmp     word 0x18:.pmode16

.pmode16:
	[bits   16]

	mov     eax, cr0
	and     al, ~1
	mov     cr0, eax

	jmp     word 00h:.rmode

global .rmode
.rmode:
	mov     ax, 0
	mov     ds, ax
	mov     ss, ax

	sti

	; mov     al, 'R'
	; out     0xE9, al

	xor     ax, ax
	mov     es, ax
	mov     fs, ax
	mov     gs, ax

	linear_to_seg_offset [bp + 12], ds, esi, si

    ; Load input registers from struct _biosregs
	mov     edi, [ds:si+0]
	mov     ebx, [ds:si+8]
	mov     edx, [ds:si+12]
	mov     ecx, [ds:si+16]
	mov     eax, [ds:si+20]
	mov     gs, [ds:si+24]
	mov     fs, [ds:si+26]
	mov     es, [ds:si+28]

	push    eax
	pushfd
	pop     eax
	or      eax, [ds:si+32]
	and     eax, ~0x0100
	push    eax
	popfd
	pop     eax

	push    eax
	mov     ax, [ds:si+30]
	mov     esi, [ds:si+4]
	mov     ds, ax
	pop     eax

	; push    ax
	; mov     al, 'I'
	; out     0xE9, al
	; pop     ax
    ; BIOS call
	db      0xCD
global .int_number
.int_number:
	db      0

	; push    ax
	; mov     al, 'D'
	; out     0xE9, al
	; pop     ax

    ; Save output registers
	push    edi
	push    ds
	pushfd

	linear_to_seg_offset [bp + 16], ds, edi, di

	mov     [ds:di+20], eax
	pop     eax
	mov     [ds:di+32], eax						; flags

	mov     [ds:di+4], esi
	mov     [ds:di+8], ebx
	mov     [ds:di+12], edx
	mov     [ds:di+16], ecx
	mov     [ds:di+24], gs
	mov     [ds:di+26], fs
	mov     [ds:di+28], es

	pop     ax
	pop     ebx
	mov     [ds:di+30], ax						; ds
	mov     [ds:di+0], ebx						; di

	cli

	mov     eax, cr0
	or      al, 1
	mov     cr0, eax

	jmp     0x08:.pmode
.pmode:
	[bits   32]
	mov     ax, 0x10
	mov     ds, ax
	mov     es, ax
	mov     fs, ax
	mov     gs, ax
	mov     ss, ax

	; mov     al, 'P'
	; out     0xE9, al
	; mov     al, 0xA
	; out     0xE9, al

    ; Restore PM state
	pop     ss
	pop     gs
	pop     fs
	pop     es
	pop     ds
	popad
	popfd
	pop     ebp
	sti
	ret

ireg_ptr:       dd      0
oreg_ptr:       dd      0

