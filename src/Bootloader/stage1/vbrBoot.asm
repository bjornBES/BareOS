bits 16

%define ENDL 0x0D, 0x0A


%define fat12 1
%define fat16 2
%define fat32 3
%define ext2  4

section .fsjump
    jmp short start
    nop

section .fsheaders

%if (FILESYSTEM == fat12) || (FILESYSTEM == fat16) || (FILESYSTEM == fat32)
    bdb_oem:                    db "MSWIN4.1"           ; 8 bytes
    bdb_bytes_per_sector:       dw 512
    bdb_sectors_per_cluster:    db 1
    bdb_reserved_sectors:       dw 1
    bdb_fat_count:              db 2
    bdb_dir_entries_count:      dw 0E0h
    bdb_total_sectors:          dw 2880                 ; 2880 * 512 = 1.44MB
    bdb_media_descriptor_type:  db 0F0h                 ; F0 = 3.5" floppy disk
    bdb_sectors_per_fat:        dw 9                    ; 9 sectors/fat
    bdb_sectors_per_track:      dw 18
    bdb_heads:                  dw 2
    bdb_hidden_sectors:         dd 0
    bdb_large_sector_count:     dd 0

    %if (FILESYSTEM == fat32)
        fat32_sectors_per_fat:      dd 0
        fat32_flags:                dw 0
        fat32_fat_version_number:   dw 0
        fat32_rootdir_cluster:      dd 0
        fat32_fsinfo_sector:        dw 0
        fat32_backup_boot_sector:   dw 0
        fat32_reserved:             times 12 db 0
    %endif

    ; extended boot record
    ebr_drive_number:           db 0                    ; 0x00 floppy, 0x80 hdd, useless
                                db 0                    ; reserved
    ebr_signature:              db 29h
    ebr_volume_id:              db 12h, 34h, 56h, 78h   ; serial number, value doesn't matter
    ebr_volume_label:           db 'BJORNBES OS'        ; 11 bytes, padded with spaces
    ebr_system_id:              db 'FAT12   '           ; 8 bytes
%endif

section .entry
    global start
start:
    ; mov ax, 0
    ; mov ds, ax
    ; mov si, 0x7dbe

    ; mov ax,     PARTITION_ENTRY_SEGMENT
    ; mov es,     ax
    ; mov di,     PARTITION_ENTRY_OFFSET
    ; mov cx,     32*2
    ; rep movsb

    ; Set up segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    
    ; Set stack pointer
    mov ss, ax
    mov sp, 0x7C00
    
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
    
.extensions_checked:
    
    mov si, stage2_location

    mov ax, STAGE2_LOAD_SEGMENT
    mov es, ax

    mov bx, STAGE2_LOAD_OFFSET
global .loop
.loop:
    mov eax, [si]
    add si, 4
    mov cl, [si]
    inc si

    cmp cl, 0
    je .read_finish

    call disk_read
    
    xor ch, ch
    shl cx, 5
    mov di, es
    add di, cx
    mov es, di
    
    jmp .loop
global .read_finish
    .read_finish:
    mov ah, 0x0e
    mov al, 'F'
    int 0x10
    mov dl, [ebr_drive_number]
    mov si, 0x7dbe
    mov di, 0


    mov ax, STAGE2_LOAD_SEGMENT
    mov ds, ax
    mov es, ax
global .Jump
.Jump:
    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET
    
section .text
no_stage2:
    mov si, msg_no_stage2
    call print_string
    hlt
    
read_error:
    mov si, msg_read_failed
    call print_string
    hlt

;
; Converts an LBA address to a CHS address
; Parameters:
;   - ax: LBA address
; Returns:
;   - cx [bits 0-5]: sector number
;   - cx [bits 6-15]: cylinder
;   - dh: head
;
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

;
; Reads sectors from a disk
; Parameters:
;   - ax: LBA address
;   - cl: number of sectors to read (up to 128)
;   - dl: drive number
;   - es:bx: memory address where to store read data
;
disk_read:

    push eax                           ; save registers we will modify
    push bx
    push cx
    push dx
    push si
    push di

    cmp byte [have_extensions], 1
    jne .no_disk_extensions

    mov [dap_lba], eax                  ; LBA address
    mov [dap_segment], es               ; Segment
    mov [dap_offset], bx                ; Offset in segment
    mov [dap_count], cl                 ; Read 1 sector at a time
    mov [dap_count + 1], byte 0         ; we dont know what ch is so it's 0
    
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

; ================================================================
; Function: print_string
; Parameters: SI = address of null-terminated string
; ================================================================
print_string:
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

section .rodata
    msg_no_stage2:      db "No stage2 to load", ENDL, 0
    msg_read_failed:    db "Failed to read stage2", ENDL, 0

section .data
    have_extensions:    db 0
    
    ; Disk Address Packet for INT 13h extension
extensions_dap:
    dap_size:           db 0x10
    dap_reserved:       db 0
    dap_count:          dw 0
    dap_offset:         dw 0x0000
    dap_segment:        dw 0x0000
    dap_lba:        dq 0

STAGE2_LOAD_SEGMENT         equ 0x0
STAGE2_LOAD_OFFSET          equ buffer

PARTITION_ENTRY_SEGMENT     equ 0x2000
PARTITION_ENTRY_OFFSET      equ 0

section .data
global stage2_location
stage2_location:
    times 15 db 0

section .bss
buffer:
    resb 512