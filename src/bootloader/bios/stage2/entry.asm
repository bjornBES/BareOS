;
; File: entry.asm
; File Created: 14 Aug 2026
; Author: BjornBEs
; -----
; Last Modified: 14 Aug 2026
; Modified By: BjornBEs
; -----
;

	[bits   16]

section .entry

extern __stack_top
extern __bss_start
extern __end

extern pre_386_entry
extern start
extern entry_64

;
; bl = cpu tier
; dl = drive number
; di:si = gpt partition offset
; cx = partition entry index
;
global stage2_entry
stage2_entry:
	cli

	mov     byte [cpu_tier], bl
	mov     byte [boot_drive], dl
	mov     short [mbr_partition_segment], di
	mov     short [mbr_partition_offset], si
	mov     short [boot_partition_index], cx

	mov     ax, ds
	mov     ss, ax
	mov     sp, 0xFFF0
	mov     bp, sp

	call    enable_A20
	mov     al, 'G'
	out     0xe9, al
	lgdt    [gdt_desc]

	mov     al, 'P'
	out     0xe9, al
	mov     eax, cr0
	or      al, 1
	mov     cr0, eax

	mov     al, 'J'
	out     0xe9, al
	jmp     dword 0x08:.pmode
global .pmode
.pmode:
	[bits 32]

	mov     ax, 0x10
	mov     ds, ax
	mov     es, ax
	mov     gs, ax
	mov     fs, ax
	mov     ss, ax

	mov     al, 'D'
	out     0xe9, al

	mov     edi, __bss_start
	mov     ecx, __end
	sub     ecx, edi
	mov     al, 0
	cld
	rep     stosb
	mov     esp, __stack_top

	mov     byte bl, [cpu_tier]
	cmp     bl, 1
	jne     .no_cpuid							; pre-386, don't even try

.detect_cpuid:

	pushfd
	pushfd
	xor     dword [esp], 0x200000				; toggle bit 21
	popfd
	pushfd
	pop     eax
	xor     eax, [esp]
	popfd
	cmp     eax, 0x200000
	jne     .no_cpuid							; didn't stick -> no CPUID (early 386/486SX-era edge case)
	mov     byte [have_cpuid], 1
	jmp     .done
.no_cpuid:
	mov     byte [have_cpuid], 0
	jmp     $
.done:

    ; call global constructors
    ; call _init

	mov     di, [mbr_partition_segment]
	shl     edi, 16
	mov     di, [mbr_partition_offset]
	push    edi

	pushfd
	pop eax
	and eax, 0xFF
	push eax
	popfd

	call    start

	jmp 	$

puts:
	push    si
	push    ax
	push    bx

.loop:
	lodsb
	or      al, al
	jz      .done


	mov     ah, 0x0E
	mov     bh, 0
	int     0x10

	jmp     .loop
.done:

	pop     bx
	pop     ax
	pop     si
	ret

global enable_A20
enable_A20:
	[bits   16]
    ; disable keyboard
	call    A20_wait_input
	mov     al, Kbd_controller_disable_keyboard
	out     Kbd_controller_command_port, al

    ; read control output port
	call    A20_wait_input
	mov     al, Kbd_controller_read_ctrl_output_port
	out     Kbd_controller_command_port, al

	call    A20_wait_output
	in      al, Kbd_controller_data_port
	push    eax

    ; write control output port
	call    A20_wait_input
	mov     al, Kbd_controller_write_ctrl_output_port
	out     Kbd_controller_command_port, al

	call    A20_wait_input
	pop     eax
	or      al, 2
	out     Kbd_controller_data_port, al

    ; enable keyboard
	call    A20_wait_input
	mov     al, Kbd_controller_enable_keyboard
	out     Kbd_controller_command_port, al

	call    A20_wait_input
	ret

A20_wait_input:
	[bits   16]

	in      al, Kbd_controller_command_port
	test    al, 2
	jnz     A20_wait_input
	ret

A20_wait_output:
	[bits   16]

	in      al, Kbd_controller_command_port
	test    al, 1
	jz      A20_wait_output
	ret

Kbd_controller_data_port equ     0x60
Kbd_controller_command_port equ     0x64
Kbd_controller_disable_keyboard equ     0xAD
Kbd_controller_enable_keyboard equ     0xAE
Kbd_controller_read_ctrl_output_port equ     0xD0
Kbd_controller_write_ctrl_output_port equ     0xD1

message:        db      "Hello world", 0xD, 0xA, 0

; 1 - Base 0 - 31
; 2 - Limit 0 - 19
; 3 - Access 0 - 7
; 4 - Flags 0 - 3
%macro gdt_descriptor 4
	dw      %2 & 0xFFFF							; 0-15 limit
	dw      %1 & 0xFFFF							; 0-15 base
	db      (%1 >> 16) & 0xFF					; 16-23 base
	db      %3									; 0-7 access
	db      (%4 << 4) | ((%2 >> 16) & 0xFF)		; 16-19 limit and 0-3 flags
	db      (%1 >> 24) & 0xFF					; 24-31 base
%endmacro

gdt:
	; descriptor 0 - null
	dq      0

	; 32-bit code segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_1_0_1_0, 0b1_1_0_0

	; 32-bit data segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_0_0_1_0, 0b1_1_0_0

	; 16-bit code segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_1_0_1_0, 0b0_0_0_0

	; 16-bit data segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_0_0_1_0, 0b0_0_0_0

%if __x86_64__=1

	; 64-bit code segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_1_0_1_0, 0b0_0_1_0

	; 64-bit data segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_0_0_1_0, 0b0_0_1_0

%endif
gdt_desc:
	dw      gdt_desc - gdt - 1
	dd      gdt

global cpu_tier
cpu_tier:
	resb    1

global have_cpuid
have_cpuid:
	resb    1

global boot_drive
boot_drive:
	resw    1

global mbr_partition_segment
mbr_partition_segment:
	resw    1

global mbr_partition_offset
mbr_partition_offset:
	resw    1

global boot_partition_index
boot_partition_index:
	resw    1
