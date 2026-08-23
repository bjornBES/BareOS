;
; File: mbr.asm
; File Created: 13 Aug 2026
; Author: BjornBEs
; -----
; Last Modified: 13 Aug 2026
; Modified By: BjornBEs
; -----
;

	[bits   16]

%include "function_table.inc"

%define ENDL 0x0D, 0x0A

section .text

extern __vbr_start

    global start
start:
	cli

	xor     ax, ax
	mov     ds, ax
	mov     es, ax

    ; setting the stack pointer
	mov     ss, ax
	mov     di, 0x600
	mov     sp, di

	mov     cx, 512
	mov     si, 0x7C00
	; di = 0x600
	rep     movsw
    ; relocating the mbr to 0x600

	jmp     0:.after
.after:
	sti

    ; Save boot drive number
	mov     byte [_drive_number], dl

    ; Check for INT 13h extension support (AH=0x41)
	mov     ah, 0x41
	mov     bx, 0x55AA
	stc
	int     0x13

	jc      .no_extensions
	cmp     bx, 0xAA55
	jc      .no_extensions

	mov     byte [_have_extensions], 1
	mov     byte [dap_size], 0x10
	mov     di, dap_reserved
	mov     cx, 15
	xor     al, al
	rep     stosb

	jmp     .extensions_checked

.no_extensions:
	mov     byte [_have_extensions], 0
	mov     ah, 0x08

	mov     dl, [_drive_number]
	xor     di, di

	clc
	int     0x13
	jnc     .geo_successful
	mov     si, msg_geo_failed
	jmp     halt
.geo_successful:

	mov     al, cl
	and     al, 0x3F
	mov     [bdb_sectors_per_track], ax

	mov     al, dh
	inc     al
	mov     [bdb_heads], ax

.extensions_checked:

	mov     si, __partition						; Offset to partition table from start of MBR
	mov     cl, 4								; 4 partition entries to check
	xor     cx, cx

global .search_partitions
.search_partitions:

    ; Check boot flag at offset 0 in partition entry
	mov     al, byte [es:si]
	cmp     al, 0x80
	je      .found_bootable

    ; Check for gpt
	mov     al, byte [es:si + 4]
	cmp     al, 0xEE
	jne     .next_partition
	mov     si, msg_no_bootable_partitions
	jmp     halt
.next_partition:

    ; Move to next partition entry (16 bytes each)
	add     si, 16
	inc     ch
	dec     cl
	jnz     .search_partitions

    ; No bootable partition found
    ; Print "N" for no bootable
	mov     al, 'N'
	out     0xe9, al
	jmp     boot_error

.found_bootable:
	mov     byte [partition_index], ch

    ; Read partition's LBA start address (at offset 8 in partition entry)
	add si, 8
	mov di, partition_lba
	movsw
	movsw
	; mov     ax, [es:si + 8]
	; mov     [partition_lba], ax
	; mov     ax, [es:si + 10]
	; mov     [partition_lba + 2], ax

    ; Read number of sectors (at offset 12 in partition entry)
	movsw
	movsw
	; mov     ax, [si + 12]
	; mov     [partition_sectors], ax
	; mov     ax, [si + 14]
	; mov     [partition_sectors + 2], ax

    ; Reset DS to 0 so DAP access works correctly
	xor     ax, ax
	mov     es, ax								; ES = 0x0000
	mov     ds, ax

    ; Set up for reading VBR
    ; Load to physical address 0x7E00 (segment 0x0000, offset 0x7E00)
	mov     bx, __vbr_start						; BX = 0x7E00 (offset within segment)

    ; Get the LBA address
	mov     ax, [partition_lba]
	mov     si, [partition_lba + 2]
	mov     dl, [_drive_number]

	mov     cl, 1
	call    read_sector_lba

global .check
.check:
    ; Check if VBR was loaded (first byte should be 0xEB for jump)
    ; Use segment override to access 0x7E00 directly
	mov     al, 'E'
	out     0xe9, al
	cmp     byte [es:bx], 0xeb
	jne     boot_error
	
	mov     al, 'S'
	out     0xe9, al
	cmp     word [es:bx + 510], 0xAA55
	jne     boot_error

	mov     si, __partition
	mov     cx, [partition_index]

	mov     word [_read_lba], read_sector_lba
	mov     word [_chs_to_lba], lba_to_chs
	mov     word [_print], print_string
	mov     word [_disk_reset], disk_reset

    ; Jump to VBR at physical 0x7E00
    ; Use far jump
	jmp     0:__vbr_start

read_error:
	mov     si, msg_read_failed
	jmp     halt

boot_error:
	mov     si, msg_no_bootable
halt:
	call    print_string

	hlt
	jmp     $

lba_to_chs:
	push    ax
	push    dx

	xor     dx, dx								; dx = 0
	div     word [bdb_sectors_per_track]		; ax = LBA / SectorsPerTrack
                                            ; dx = LBA % SectorsPerTrack

	inc     dx									; dx = (LBA % SectorsPerTrack + 1) = sector
	mov     cx, dx								; cx = sector

	xor     dx, dx								; dx = 0
	div     word [bdb_heads]					; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                            ; dx = (LBA / SectorsPerTrack) % Heads = head
	mov     dh, dl								; dh = head
	mov     ch, al								; ch = cylinder (lower 8 bits)
	shl     ah, 6
	or      cl, ah								; put upper 2 bits of cylinder in CL

	pop     ax
	mov     dl, al
	pop     ax
	ret

; ============================================================================
; Function: read_sector_lba
; Parameters:
;   ax:si = 32 bit LBA address
;   cl = number of sectors to read (up to 128)
;   dl = drive number
;   es:bx = destination (es = segment, bx = offset)
; ============================================================================
global read_sector_lba
read_sector_lba:

	; save registers we will modify nearly them all
	pusha

	cmp     byte [_have_extensions], 1
	jne     .no_disk_extensions

    ; with extensions
	mov     [dap_lba], ax
	mov     [dap_lba + 2], si
	mov     dword [dap_lba + 4], 0
	mov     [dap_segment], es
	mov     [dap_offset], bx
	mov     [dap_count], cl						; Read 1 sector at a time
	mov     [dap_count + 1], byte 0				; we dont know what ch is so it's 0

	mov     ah, 0x42
	mov     si, extensions_dap
	mov     di, 3								; retry count
	jmp     .retry

.no_disk_extensions:
	push    cx									; temporarily save CL (number of sectors to read)
	call    lba_to_chs							; compute CHS
	pop     ax									; AL = number of sectors to read

	mov     ah, 02h
	mov     di, 3								; retry count

.retry:
	pusha   									; save all registers, we don't know what bios modifies
	stc     									; set carry flag, some BIOS'es don't set it
	int     013h								; carry flag cleared = success
	jnc     .done								; jump if carry not set

    ; read failed
	popa
	call    disk_reset

	dec     di
	test    di, di
	jnz     .retry

.fail:
    ; all attempts are exhausted
	jmp     read_error

.done:
	popa

	popa
	ret

;
; Resets disk controller
; Parameters:
;   dl: drive number
;
disk_reset:
	pusha
	mov     ah, 0
	stc
	int     013h
	jc      read_error
	popa
	ret

; ============================================================================
; Function: print_string
; Parameters: SI = address of null-terminated string
; ============================================================================
global print_string
print_string:
	push    ax
	push    bx
	cld

.print_loop:
	lodsb
	test    al, al
	jz      .print_done

	out     0xe9, al
	mov     ah, 0x0e
	mov     bx, 7
	int     0x10

	jmp     .print_loop

.print_done:
	pop     bx
	pop     ax
	ret

section .rodata
    ; no bootable disk found
global msg_no_bootable
msg_no_bootable:
	db      "NOBOD", 0
	; No Bootable Disks

    ; no bootable disk found
global msg_no_bootable_partitions
msg_no_bootable_partitions:
	db      "NOBOP", 0
	; No Bootable Partitions

    ; Failed read VBR
global msg_read_failed
msg_read_failed:
	db      "FARE", 0
	; Failed Read

    ; Failed to get drive geo
global msg_geo_failed
msg_geo_failed:
	db      "FTP", 0
	; Failed To get Parameters

section .UDI

	db      0xFF, 0xFF, 0xFF, 0xFF

section .partition
__partition:
	times   64 db 0xFF

section .bios_footer

section .bss
partition_lba:
	resd    1
partition_sectors:
	resd    1
bdb_sectors_per_track:
	resw    1
bdb_heads:
	resb    1
partition_index:
	resw    1
buffer:
	resb    512