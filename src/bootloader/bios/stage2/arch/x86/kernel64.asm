;
; File: kernel64.asm
; File Created: 30 Aug 2026
; Author: BjornBEs
; -----
; Last Modified: 30 Aug 2026
; Modified By: BjornBEs
; -----
;

	[bits   32]

extern _cr3

;
; void x86_enable64(boot_params_t *boot_params, uint64 entry);
;
; boot_params_t * - pushed as a uint64
; uint64 - pushed as a uint64
;
global x86_enable64
x86_enable64:
	push    ebp
	mov     ebp, esp

	mov     ecx, 0xC0000080
	rdmsr
	or      eax, 1 << 8
	wrmsr
	mov     al, 'L'
	out     0xe9, al

	mov     eax, [_cr3]
	mov     cr3, eax
	mov     al, 'P'
	out     0xe9, al

global .enable_paging
.enable_paging:
	mov     eax, cr0
global .here
.here:
	or      eax, 1 << 31
global .here1
.here1:
	mov     cr0, eax
global .here2
.here2:
	mov     al, 'E'
	out     0xe9, al

	jmp     0x28:.reload_section
	[bits   64]
global .reload_section
.reload_section:

	mov     ax, 0x30							; data selector (offset 0x30)
	mov     ds, ax
	mov     es, ax
	mov     fs, ax
	mov     gs, ax

	lgdt    [gdt_desc_64]
	mov     al, 'G'
	out     0xe9, al

	mov     rdi, .reload
	push    0x8
	push    rdi
	retfq
global .reload
.reload:

	mov     al, 'R'
	out     0xe9, al

	mov     ax, 0x10							; data selector
	mov     ds, ax
	mov     es, ax
	mov     fs, ax
	mov     gs, ax
	mov     ss, ax

	mov     al, 'J'
	out     0xe9, al
	mov     al, 0xa
	out     0xe9, al
	mov     rdi, [ebp + 8]
	mov     rax, [ebp + 16]
	jmp     rax

gdt_64:
    ; Null Descriptor
	dq      0

    ; 64-bit code segment
	dw      0xFFFF								; limit (bits 0-15)
	dw      0									; base (bits 0-15)
	db      0									; base (bits 16-23)
	db      10011010b							; access
	db      00101111b							; limit (bits 16-19) + flags
	db      0									; base (bits 24-31)

    ; 64-bit data segment
	dw      0xFFFF								; limit (bits 0-15)
	dw      0									; base (bits 0-15)
	db      0									; base (bits 16-23)
	db      10010010b							; access
	db      00101111b							; limit (bits 16-19) + flags
	db      0									; base (bits 24-31)

gdt_desc_64:
	dw      gdt_desc_64 - gdt_64 - 1
	dq      gdt_64