;
; File: smp_trampoline.asm
; File Created: 08 May 2026
; Author: BjornBEs
; -----
; Last Modified: 08 May 2026
; Modified By: BjornBEs
; -----
;

;
; AP trampoline — copied to TRAMPOLINE_PHYS (0x8000) at runtime.
; APs wake in 16-bit real mode and need to reach 64-bit long mode
; as fast as possible, then jump to ap_startup() in C.
;
; Memory layout at 0x8000:
;   [+0x00]  jmp over data (2 bytes)
;   [+0x02]  trampoline_data_t  (4 x uint64 = 32 bytes)
;   [+0x22]  smp_trampoline_start (actual 16-bit code)
;
; trampoline_data_t (filled by BSP before SIPI):
;   uint64_t kernel_cr3    — physical address of kernel PML4
;   uint64_t cpu_stack     — top of this AP's kernel stack
;   uint64_t kernel_gdt    — linear address of GDTR descriptor (6 bytes)
;   uint64_t ap_entry      — virtual address of ap_startup()
;


	[bits   16]
section .smp_trampoline_text
; ov shit here we go again
; i haven't missed 16 bit at all
global smp_trampoline_boot
smp_trampoline_boot:
	jmp     smp_trampoline_start				; eb 20

global trampoline_kernel_cr3
global trampoline_cpu_stack
global trampoline_kernel_gdt
global trampoline_ap_entry

trampoline_kernel_cr3: dq      0
trampoline_cpu_stack: dq      0
trampoline_kernel_gdt: dq      0
trampoline_ap_entry: dq      0

smp_trampoline_start:
	[bits   16]
	cli     									; fa
	xor     ax, ax								; 31 c0
	mov     ds, ax								; 8e d8

	mov     al, '0'
	out     0xE9, al
	mov     al, '8'
	out     0xE9, al
	mov     al, 'b'
	out     0xE9, al
    ; we are now in 16 bit code welcome
    ; we wont be here for long
    ; i fucking hate this

    ; goal here get to long as soon
    ; as fast as fucking possible

	lgdt    [real_mode_gdt_desc]				; 0f 01 16 48 01

	mov     eax, cr0							; 0f 20 c0
	or      al, 1								; 0c 01
	mov     cr0, eax							; 0f 22 c0

	jmp     dword 0x18:the_better_mode			; 66 ea 3c 00 00 00 18 00
the_better_mode:
	[bits   32]

    ; that was the easy thing
    ; now comes the long part
	mov     di, 0x10							; 66 bf 10 00
	mov     ax, 0x20							; 66 b8 20 00
	mov     ds, ax								; 8e d8
	mov     ss, ax								; 8e d0

	mov     al, '3'
	out     0xE9, al
	mov     al, '2'
	out     0xE9, al
	mov     al, 'b'
	out     0xE9, al

	xor     ebp, ebp							; 31 ed

	mov     eax, dword [trampoline_kernel_cr3]	; a1 02 00 00 00
	mov     cr3, eax							; 0f 22 d8
	mov     esi, 0x80000001						; be 01 00 00 80
	mov     eax, 0x80000000						; b8 00 00 00 80

	cpuid   									; 0f a2

	cmp     eax, esi							; 39 f0
	jb      jump_kernel32						; 72 0a

	mov     eax, esi							; 89 f0
	cpuid   									; 0f a2

	; using bt instead of cmp, bc cmp takes 6 bytes!!
	bt      edx, 29								; 0f ba e2 1d
	jc      enable_long							; 72 36
jump_kernel32:

	lgdt    [trampoline_kernel_gdt]				; 0f 01 15 12 00 00 00

	mov     esp, [trampoline_cpu_stack]			; 8b 25 0a 00 00 00

	mov     eax, cr0							; 0f 20 c0
	bts     eax, 31								; 0f ba e8 1f
	; instead of "or eax, 1 << 31"
	; which is 0d 00 00 00 80
	mov     cr0, eax							; 0f 22 c0

	jmp     dword 0x08:the_2nd_best_mode		; ea 8c 00 00 00 08 00
the_2nd_best_mode:
	mov     ds, di								; 8e df
	mov     es, di								; 8e c7
	mov     fs, di								; 8e e7
	mov     gs, di								; 8e ef
	mov     ss, di								; 8e d7

	jmp     [trampoline_ap_entry]				; ff 25 1a 00 00 00

enable_long:
    ; Enabling Physical Address Extension and Page Size Extension
	mov     eax, cr4							; 0f 20 e0
	or      al, (1 << 5) | (1 << 4)				; 0c 30
	mov     cr4, eax							; 0f 22 e0

	xchg    edx, esi							; 87 d6
	mov     ecx, 0xC0000080						; b9 80 00 00 c0
	rdmsr   									; 0f 32
	or      ah, (1 << 0)						; 80 cc 01
	bt      esi, 20								; 0f ba e6 14
	jnc     .no_nxe								; 73 04
	or      ah, (1 << 3)						; 80 cc 08

.no_nxe:
	wrmsr   									; 0f 30

    ; fuck yes 64 bits fuck yes
	mov     eax, cr0							; 0f 20 c0
	bts     eax, 31								; 0f ba e8 1f
	; instead of "or eax, 1 << 31"
	; which is 0d 00 00 00 80
	mov     cr0, eax							; 0f 22 c0

	jmp     dword 0x08:the_best_mode			; ea cd 00 00 00 08 00
the_best_mode:
	[bits   64]

	mov     ds, di								; 8e df
	mov     es, di								; 8e c7
	mov     fs, di								; 8e e7
	mov     gs, di								; 8e ef
	mov     ss, di								; 8e d7

	mov     al, '6'
	out     0xE9, al
	mov     al, '4'
	out     0xE9, al
	mov     al, 'b'
	out     0xE9, al

	xor     rax, rax
	inc     eax
	cpuid
	bt      ecx, 21
	jnc		.no_x2apic

	mov     ecx, 0x1b
	rdmsr
	or      ah, (1 << 2) | (1 << 3)
	wrmsr
.no_x2apic:

	lgdt    [trampoline_kernel_gdt]				; 0f 01 14 25 12 00 00 00

	mov     rsp, [trampoline_cpu_stack]			; 48 8b 24 25 0a 00 00 00
	xor     rbp, rbp							; 48 31 ed

	mov     rax, [trampoline_ap_entry]			; 48 8b 04 25 1a 00 00 00
	jmp     rax									; ff e0


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

align 8
real_mode_gdt:
    ; Null Descriptor
null_descriptor:
	dq      0

_64_code_descriptor:
	; 64-bit code segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_1_0_1_0, 0b0_0_1_0

_64_data_descriptor:
	; 64-bit data segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_0_0_1_0, 0b0_0_1_0

_32_code_descriptor:
	; 32-bit code segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_1_0_1_0, 0b1_1_0_0

_32_data_descriptor:
	; 32-bit data segment
	gdt_descriptor 0, 0xFFFFF, 0b1_00_1_0_0_1_0, 0b1_1_0_0

global real_mode_gdt_desc
real_mode_gdt_desc: dw      real_mode_gdt_desc - real_mode_gdt - 1
	dd      real_mode_gdt

global smp_trampoline_end
smp_trampoline_end:
