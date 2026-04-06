;
; File: entry.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 11 Mar 2026
; Modified By: BjornBEs
; -----
;

bits 16

section .entry

%define ENDL 0x0D, 0x0A

extern __bss_start
extern __end
extern _init

extern start
extern entry_64
global entry

entry:
    cli

    mov [BootDrive], dl
    mov [BootPartitionOff], si
    mov [BootPartitionSeg], di
    
    mov si, message
    call puts

    mov ax, ds
    mov ss, ax
    mov sp, 0xFFF0
    mov bp, sp

    ; get_time scope
    mov         ah,             0x02
    clc
    ; CF clear to avoid bug
    int         0x1A                            ; TIME - GET REAL-TIME CLOCK TIME
    ; Return:
    ; CF clear if successful
    ; CH = hour (BCD)
    ; CL = minutes (BCD)
    ; DH = seconds (BCD)
    ; DL = daylight savings flag (00h standard time, 01h daylight time)
    ; CF set on error (i.e. clock not running or in middle of update)
    ;
    
    jc          .get_time_failed                ; there was an error
    ; if the year is 8230 that is not a bug that is a feature.
    mov         ebx,            0x00200007      ; boot params RTC_info
    mov         eax,            ebx
    mov         [eax],          ch              ; set hour
    mov         [eax + 1],      cl              ; set minute
    mov         [eax + 2],      dh              ; set second

    mov         ah,             0x04
    clc
    ; CF clear to avoid bug
    int         0x1A                            ; TIME - GET REAL-TIME CLOCK DATE
    ; Return:
    ; CF clear if successful
    ; CH = century (BCD)
    ; CL = year (BCD)
    ; DH = month (BCD)
    ; DL = day (BCD)
    ; CF set on error (i.e. clock not running or in middle of update)
    ;
    
    jc          .get_time_failed                ; there was an error
    mov         eax,            ebx
    mov         [eax + 3],      dl              ; set day
    mov         [eax + 4],      dh              ; set month
    mov         [eax + 5],      cx              ; set year
    jmp         .get_time_end

.get_time_failed:
    mov dx, 0xE9
    mov al, 'E'
    out dx, al
    ; print error or something
.get_time_end:

    call EnableA20
    call LoadGDT

    mov eax, cr0
    or al, 1
    mov cr0, eax

    
    jmp dword 0x08:.pmode

.pmode:
    [bits 32]

    mov ax, 0x10
    mov ds, ax
    mov ss, ax

    mov edi, __bss_start
    mov ecx, __end
    sub ecx, edi
    mov al, 0
    cld
    rep stosb

    ; call global constructors
    call _init

    mov di, [BootPartitionSeg]
    shl edi, 16
    mov di, [BootPartitionOff]
    push edi
    
    xor esi, esi
    mov si, [BootDrive]

    ; check long mode
    mov eax, 0x80000001
    cpuid
    and edi, (1 << 29)
    jz ._32bit   
    call entry_64
    jmp ._halt
._32bit:
    push edi
    push esi 

    call start
._halt:
    cli
    hlt

message: db "Hello world", 0xD, 0xA, 0

puts:
    push si
    push ax
    push bx

.loop:
    lodsb
    or al, al
    jz .done
    

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    jmp .loop
.done:

    pop bx
    pop ax
    pop si
    ret

EnableA20:
    [bits 16]
    ; disable keyboard
    call A20WaitInput
    mov al, KbdControllerDisableKeyboard
    out KbdControllerCommandPort, al
    
    ; read control output port
    call A20WaitInput
    mov al, KbdControllerReadCtrlOutputPort
    out KbdControllerCommandPort, al

    call A20WaitOutput
    in al, KbdControllerDataPort
    push eax

    ; write control output port
    call A20WaitInput
    mov al, KbdControllerWriteCtrlOutputPort
    out KbdControllerCommandPort, al

    call A20WaitInput
    pop eax
    or al, 2
    out KbdControllerDataPort, al

    ; enable keyboard
    call A20WaitInput
    mov al, KbdControllerEnableKeyboard
    out KbdControllerCommandPort, al

    call A20WaitInput
    ret

A20WaitInput:
    [bits 16]

    in al, KbdControllerCommandPort
    test al, 2
    jnz A20WaitInput
    ret

A20WaitOutput:
    [bits 16]

    in al, KbdControllerCommandPort
    test al, 1
    jz A20WaitOutput
    ret

KbdControllerDataPort               equ 0x60
KbdControllerCommandPort            equ 0x64
KbdControllerDisableKeyboard        equ 0xAD
KbdControllerEnableKeyboard         equ 0xAE
KbdControllerReadCtrlOutputPort     equ 0xD0
KbdControllerWriteCtrlOutputPort    equ 0xD1

    
LoadGDT:
    [bits 16]
    lgdt [g_GDTDesc]
    ret

%macro GDTDescriptor 6
    dw %1
    dw %2
    db %3
    db %4
    db %5
    db %6
%endmacro

g_GDT:
    ; Null Descriptor
    dq 0

    ; 32-bit code segment
    GDTDescriptor 0xFFFF, 0, 0, 10011010b, 11001111b, 0
    
    ; 32-bit data segment
    GDTDescriptor 0xFFFF, 0, 0, 10010010b, 11001111b, 0

    ; 16-bit code segment
    GDTDescriptor 0xFFFF, 0, 0, 10011010b, 00001111b, 0
    
    ; 16-bit data segment
    GDTDescriptor 0xFFFF, 0, 0, 10010010b, 00001111b, 0

    ; 64-bit code segment
    GDTDescriptor 0xFFFF, 0, 0, 10011010b, 00100000b, 0
    
    ; 64-bit data segment
    GDTDescriptor 0xFFFF, 0, 0, 10010010b, 00100000b, 0

g_GDTDesc:  dw g_GDTDesc - g_GDT - 1
            dd g_GDT

g_BootDrive: dw 0
g_BootPartitionSeg: dw 0
g_BootPartitionOff: dw 0
