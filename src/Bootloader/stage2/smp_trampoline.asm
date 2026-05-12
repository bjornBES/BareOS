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


[bits 16]
section .smp_trampoline_text
; ov shit here we go again
; i haven't missed 16 bit at all
global smp_trampoline_boot
smp_trampoline_boot:
    jmp smp_trampoline_start

global trampoline_kernel_cr3
global trampoline_cpu_stack
global trampoline_kernel_gdt
global trampoline_ap_entry
 
trampoline_kernel_cr3:  dq 0
trampoline_cpu_stack:   dq 0
trampoline_kernel_gdt:  dq 0
trampoline_ap_entry:    dq 0

smp_trampoline_start:
    [bits 16]
    cli
    xor ax, ax
    mov ds, ax

    ; we are now in 16 bit code welcome
    ; we wont be here for long
    ; i fucking hate this

    ; goal here get to long as soon
    ; as fast as fucking possible
    
    lgdt [real_mode_gdt_desc]

    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp dword 0x18:the_better_mode
the_better_mode:
    [bits 32]

    ; that was the easy thing
    ; now comes the long part
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov dx, 0xE9
    mov al, '3'
    out dx, al
    mov al, '2'
    out dx, al
    mov al, 'b'
    out dx, al

    ; Enabling Physical Address Extension and Page Size Extension 
    mov eax, cr4
    or eax, (1 << 5) | (1 << 4)
    mov cr4, eax

    mov eax, dword [trampoline_kernel_cr3]
    mov cr3, eax

    
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8) | (1 << 11) | (1 << 0)
    wrmsr

    ; fuck yes 64 bits fuck yes
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    
    jmp dword 0x08:the_best_mode
the_best_mode:
    [bits 64]
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov dx, 0xE9
    mov al, '6'
    out dx, al
    mov al, '4'
    out dx, al
    mov al, 'b'
    out dx, al

    lgdt [trampoline_kernel_gdt]

    mov rsp, [trampoline_cpu_stack]
    xor rbp, rbp
    
    xor rdi, rdi
    mov eax, 1
    cpuid
    shr ebx, 24
    mov edi, ebx

    mov rax, [trampoline_ap_entry]
    jmp rax


%macro GDTDescriptor 6
    dw %1
    dw %2
    db %3
    db %4
    db %5
    db %6
%endmacro

align 8
real_mode_gdt:
    ; Null Descriptor
    dq 0
    
    ; 64-bit code segment
    GDTDescriptor 0xFFFF, 0, 0, 10011010b, 00100000b, 0
    
    ; 64-bit data segment
    GDTDescriptor 0xFFFF, 0, 0, 10010010b, 00100000b, 0

    ; 32-bit code segment
    GDTDescriptor 0xFFFF, 0, 0, 10011010b, 11001111b, 0
    
    ; 32-bit data segment
    GDTDescriptor 0xFFFF, 0, 0, 10010010b, 11001111b, 0
    
real_mode_gdt_desc:     dw real_mode_gdt_desc - real_mode_gdt - 1
                        dd real_mode_gdt

global smp_trampoline_end
smp_trampoline_end:
