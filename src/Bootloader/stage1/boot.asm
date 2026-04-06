bits 16

%define ENDL 0x0D, 0x0A

section .entry
    global start
start:
    ; move partition entry from MBR to a different location so we
    ; don't overwrite it
    ; passed through DS:SI
    mov ax,     PARTITION_ENTRY_SEGMENT
    mov es,     ax
    mov di,     PARTITION_ENTRY_OFFSET
    mov cx,     32
    rep movsb
    
    mov ax, 0
    mov ds, ax
    mov es, ax
    
    mov ss,ax
    mov sp, 0x7C00              ; stack grows down
    
    push es
    push word .after
    retf
.after:
    mov [ebr_drive_number], dl
    
    mov ah, 0x41
    mov bx, 0x55AA
    stc
    int 13h
    
    jc .no_disk_ext
    cmp bx, 0xAA55
    jne .no_disk_ext

    mov byte [have_extensions], 1
    jmp .after_disk_ext_check
    
.no_disk_ext:
    mov byte [have_extensions], 0
    
.after_disk_ext_check:
    mov si, stage2_location

    mov ax, STAGE2_LOAD_SEGMENT
    mov es, ax

    mov bx, STAGE2_LOAD_OFFSET
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
.read_finish:
    
    mov dl, [ebr_drive_number]
    mov si, PARTITION_ENTRY_OFFSET
    mov di, PARTITION_ENTRY_SEGMENT

    mov ax, STAGE2_LOAD_SEGMENT
    mov ds, ax
    mov es, ax
    
    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

    jmp wait_and_reboot

section .text

floppy_error:
    mov si, msg_read_failed
    jmp next_jump
stage2_not_found_error:
    mov si, msg_stage2_not_found
next_jump:
    call puts

wait_and_reboot:
    mov ah, 0
    int 16h
    jmp 0x0FFFF:0           ; jump to BIOS again

.halt:
    cli
    hlt

; 
; Prints a char to the screen
; - al has char
puts:
    ; push si
    ; push ax
    ; push bx

.loop:
    lodsb
    or al, al
    jz .done
    

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    jmp .loop
.done:

    ; pop bx
    ; pop ax
    ; pop si
    ret

;
; Disk routines
;

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
;   - eax: LBA address
;   - cl: number of sectors to read (up to 128)
;   - dl: drive number
;   - es:bx: memory address where to store read data
;
disk_read:

    push eax                            ; save registers we will modify
    push bx
    push cx
    push dx
    push si
    push di

    cmp byte [have_extensions], 1
    jne .no_disk_extensions

    ; with extensions
    mov [extensions_dap.lba], eax
    mov [extensions_dap.segment], es
    mov [extensions_dap.offset], bx
    mov [extensions_dap.count], cl

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
    jmp floppy_error

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
    jc floppy_error
    popa
    ret

section .partition

status:
    db 0x80

chsFirst:
    db 0
    db 0
    db 0

type:
    db 0x0B

chsLast:
    db 0
    db 0
    db 0

lbaFirst:
    dd 0

sectors:
    dd 20480

section .rodata

    msg_read_failed:          db 'FR', ENDL, 0
    msg_stage2_not_found:     db 'S2NF', ENDL, 0

section .partition

section .data
        
have_extensions:            db 0
extensions_dap:
    .size:                  db 10h
                            db 0
    .count:                 dw 0
    .offset:                dw 0
    .segment:               dw 0
    .lba:                   dq 0

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