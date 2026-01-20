[bits 32]
section .bios

%macro x86_EnterRealMode 0
    [bits 32]
    jmp word 18h:.pmode16         ; 1 - jump to 16-bit protected mode segment

.pmode16:
    [bits 16]
    ; 2 - disable protected mode bit in cr0
    mov eax, cr0
    and al, ~1
    mov cr0, eax

    ; 3 - jump to real mode
    jmp word 00h:.rmode

.rmode:
    ; 4 - setup segments
    mov ax, 0
    mov ds, ax
    mov ss, ax

    ; 5 - enable interrupts
    sti

%endmacro


%macro x86_EnterProtectedMode 0
    cli

    ; 4 - set protection enable flag in CR0
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; 5 - far jump into protected mode
    jmp dword 08h:.pmode


.pmode:
    ; we are now in protected mode!
    [bits 32]
    
    ; 6 - setup segment registers
    mov ax, 0x10
    mov ds, ax
    mov ss, ax

%endmacro

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

global intcall
intcall:
    pushfd
    pushad
    push ds
    push es
    push fs
    push gs

	; Arguments (cdecl)
    ; [esp+...]
    mov eax, [esp+36]     ; int_no
    mov ebx, [esp+40]     ; ireg*
    mov ecx, [esp+44]     ; oreg*

    mov [.int_number], al
    mov [ireg_ptr], ebx
    mov [oreg_ptr], ecx
    
    x86_EnterRealMode

    xor ax, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Load input registers
    mov si, [ireg_ptr]
    ; Load input registers from struct _biosregs
    mov di, [si+0]
    mov bp, [si+6]
    mov bx, [si+8]
    mov dx, [si+10]
    mov cx, [si+12]
    mov ax, [si+14]
    mov gs, [si+16]
    mov fs, [si+18]
    mov es, [si+20]
    mov ds, [si+22]
    
    pushf
    pop ax
    or ax, [si+24]
    push ax
    popf
    mov si, [si+4]

    ; BIOS call
    db 0xCD
.int_number:
    db 0


    ; Save output registers
    push di
    mov di, [oreg_ptr]
    mov [di+4], si
    mov [di+6], bp
    mov [di+8], bx
    mov [di+10], dx
    mov [di+12], cx
    mov [di+14], ax
    mov [di+16], gs
    mov [di+18], fs
    mov [di+20], es
    mov [di+22], ds
    pushf
    pop ax
    mov [di+24], ax
    pop ax
    mov [di+0], ax



    x86_EnterProtectedMode

	mov ax, 0x10
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore PM state
    pop gs
    pop fs
    pop es
    pop ds
    popad
    popfd
    sti
	ret

ireg_ptr: dd 0
oreg_ptr: dd 0
