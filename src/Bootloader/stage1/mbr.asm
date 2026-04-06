;
; File: mbr.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 15 Mar 2026
; Modified By: BjornBEs
; -----
;
[bits 16]

%define ENDL 0x0D, 0x0A

%define fat12 1
%define fat16 2
%define fat32 3
%define ext2  4

section .entry

global start
start:
   
    ; Set up segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    ; mov fs, ax
    ; mov gs, ax
    
    ; Set stack pointer
    mov ss, ax
    mov sp, 0x7000
    
    push es
    push word .after
    retf
.after:
    ; Save boot drive number
    mov byte [ebr_drive_number], dl
    
    ; Check for INT 13h extension support (AH=0x41)
    mov ah, 0x41
    mov bx, 0x55AA
    stc
    int 0x13
    
    jc .no_extensions               ; If carry set, extensions not supported
    cmp bx, 0xAA55
    jne .no_extensions              ; If BX != 0xAA55, extensions not supported
    
    mov byte [have_extensions], 1
    jmp .extensions_checked
    
.no_extensions:
    mov byte [have_extensions], 0
    
    mov ah, 0x08
    mov dl, [ebr_drive_number]
    or dl, 0x80
    xor di, di
    clc
    int 0x13
    jc geometry_failed

    mov al, cl
    and al, 0x3F
    mov [bdb_sectors_per_track], ax

    mov al, dh
    inc al
    mov [bdb_heads], ax

.extensions_checked:
    
    ; mov ax, 0x07C0
    ; mov es, ax
    mov si, __partition                 ; Offset to partition table from start of MBR
    mov cx, 4                     ; 4 partition entries to check
    
.search_partitions:
    
    ; Check boot flag at offset 0 in partition entry
    mov al, byte [es:si]
    cmp al, 0x80
    je .found_bootable
    
    ; Move to next partition entry (16 bytes each)
    add si, 16
    dec cx
    jnz .search_partitions
    
    ; No bootable partition found
    ; Print "N" for no bootable
    jmp boot_error

.found_bootable:
    ; Print "B" for bootable partition found
    
    ; Read partition's LBA start address (at offset 8 in partition entry)
    mov eax, [es:si + 8]
    mov [partition_lba], eax
    
    ; Read number of sectors (at offset 12 in partition entry)
    mov eax, [si + 12]
    mov [partition_sectors], eax
    
    ; Reset DS to 0 so DAP access works correctly
    mov ax, 0
    mov es, ax                      ; ES = 0x0000
    xor ax, ax
    mov ds, ax
    
    ; Set up for reading VBR
    ; Load to physical address 0x7E00 (segment 0x0000, offset 0x7E00)
    mov bx, buffer                ; BX = 0x7E00 (offset within segment)
    
    ; Get the LBA address
    mov ax, [partition_lba]
    mov dl, [ebr_drive_number]
    
    call read_sector_lba

    ; Check if VBR was loaded (first byte should be 0xEB for jump)
    ; Use segment override to access 0x7E00 directly
    cmp byte [es:bx], 0xeb
    jne vbr_invalid
    
    mov si, __partition
    
    ; Jump to VBR at physical 0x7E00
    ; Use far jump with retf (return far)
global END
    END:
    jmp 0:buffer

section .text

geometry_failed:
    ; Print "G" for error
    mov al, 'G'
    jmp halt

read_error:
    ; Print "E" for error
    mov al, 'R'
    jmp halt

boot_error:
    ; Print "E" for error
    mov al, 'B'
    jmp halt

vbr_invalid:
    ; Print "I" for invalid VBR
    mov al, 'I'
    
halt:
    mov ah, 0x0e
    int 0x10

._halt:
    ; Print "H" for halt
    mov ah, 0x0e
    mov al, 'H'
    int 0x10
    
    hlt
    jmp ._halt

lba_to_chs:
    push ax
    push dx

    xor dx, dx                              ; dx = 0
    div word [bdb_sectors_per_track]        ; ax = LBA / SectorsPerTrack
                                            ; dx = LBA % SectorsPerTrack

    inc dx                                  ; dx = (LBA % SectorsPerTrack + 1) = sector
    mov cx, dx                              ; cx = sector

    xor dx, dx                              ; dx = 0
    div word [bdb_heads]                    ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                            ; dx = (LBA / SectorsPerTrack) % Heads = head
    mov dh, dl                              ; dh = head
    mov ch, al                              ; ch = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah                               ; put upper 2 bits of cylinder in CL

    pop ax
    mov dl, al
    pop ax
    ret

; ============================================================================
; Function: read_sector_lba
; Parameters: 
;   EAX = LBA address
;   DL = drive number
;   ES:BX = destination (ES = segment, BX = offset)
; ============================================================================
global read_sector_lba
read_sector_lba:

    push eax                             ; save registers we will modify
    push bx
    push cx
    push dx
    push si
    push di

    cmp byte [have_extensions], 1
    jne .no_disk_extensions

    ; with extensions
    mov [dap_lba_low], ax
    mov [dap_segment], es
    mov [dap_offset], bx
    mov [dap_count], word 1

    mov ah, 0x42
    mov si, extensions_dap
    mov di, 3                           ; retry count
    jmp .retry

.no_disk_extensions:
    push cx                             ; temporarily save CL (number of sectors to read)
    call lba_to_chs                     ; compute CHS
    pop ax                              ; AL = number of sectors to read
    
    mov ah, 02h
    mov di, 3                           ; retry count

.retry:
    pusha                               ; save all registers, we don't know what bios modifies
    stc                                 ; set carry flag, some BIOS'es don't set it
    int 13h                             ; carry flag cleared = success
    jnc .done                           ; jump if carry not set

    ; read failed
    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    ; all attempts are exhausted
    jmp read_error

global .done
.done:
    popa

    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop eax                            ; restore registers modified
    ret

;
; Resets disk controller
; Parameters:
;   dl: drive number
;
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc read_error
    popa
    ret

; ============================================================================
; Function: print_string
; Parameters: SI = address of null-terminated string
; ============================================================================
print_string:
    push ax
    push bx
    
.print_loop:
    lodsb
    jz .print_done
    
    mov ah, 0x0e
    mov bx, 7
    int 0x10
    
    inc si
    jmp .print_loop
    
.print_done:
    pop bx
    pop ax
    ret

section .rodata
    ; no bootable disk found
    ; msg_no_bootable:    db "No BP", ENDL, 0
    
    ; Failed read VBR
    ; msg_read_failed:    db "FR VBR", ENDL, 0

section .data
    partition_lba:          dd 0
    partition_sectors:      dd 0
    have_extensions:        db 0
    ebr_drive_number:       db 0
    bdb_sectors_per_track:  dw 18
    bdb_heads:              db 2
    
    extensions_dap:
    ; Disk Address Packet for INT 13h extension
    dap_size:           db 0x10
    dap_reserved:       db 0
    dap_count:          dw 1
    dap_offset:         dw 0x0000
    dap_segment:        dw 0x0000
    dap_lba_low:        dd 0
    dap_lba_high:       dd 0


section .partition
__partition:

section .bios_footer

section .bss
buffer:
    resb 512
