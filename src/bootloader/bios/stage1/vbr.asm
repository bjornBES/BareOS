;
; File: vbr.asm
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

%define fat12 1
%define fat16 2
%define fat32 3
%define ext2  4

section .fsjump
	jmp     short start
	nop

section .fsheaders


%if (FILESYSTEM == fat12) || (FILESYSTEM == fat16) || (FILESYSTEM == fat32)
bdb_oem:
	db      "MSWIN4.1"							; 8 bytes
bdb_bytes_per_sector:
	dw      512
bdb_sectors_per_cluster:
	db      1
bdb_reserved_sectors:
	dw      1
bdb_fat_count:
	db      2
bdb_dir_entries_count:
	dw      0E0h
bdb_total_sectors:
	dw      2880								; 2880 * 512 = 1.44MB
bdb_media_descriptor_type:
	db      0F0h								; F0 = 3.5" floppy disk
bdb_sectors_per_fat:
	dw      9									; 9 sectors/fat
bdb_sectors_per_track:
	dw      18
bdb_heads:
	dw      2
bdb_hidden_sectors:
	dd      0
bdb_large_sector_count:
	dd      0

    %if (FILESYSTEM == fat32)
fat32_sectors_per_fat:
	dd      0
fat32_flags:
	dw      0
fat32_fat_version_number:
	dw      0
fat32_rootdir_cluster:
	dd      0
fat32_fsinfo_sector:
	dw      0
fat32_backup_boot_sector:
	dw      0
fat32_reserved:
	times   12 db 0
    %endif

    ; extended boot record
ebr_drive_number:
	db      0									; 0x00 floppy, 0x80 hdd, useless
	db      0									; reserved
ebr_signature:
	db      0x29
ebr_volume_id:
	db      0x12, 0x34, 0x56, 0x78				; serial number, value doesn't matter
ebr_volume_label:
	db      'BJORNBES OS'						; 11 bytes, padded with spaces
ebr_system_id:
	db      'FAT12   '							; 8 bytes
%endif

section .entry
    global start
start:
    ; Set up segment registers
	xor     ax, ax
	mov     ds, ax
	mov     es, ax

    ; Set stack pointer
	mov     ss, ax
	mov     sp, 0x7C00

    ; Save boot drive number
	mov     byte [ebr_drive_number], dl
	mov     word [partition_offset], si
	mov     word [partition_entry_index], cx

	mov     di, stage2_location

	mov     ax, STAGE2_LOAD_SEGMENT
	mov     es, ax

	mov     bx, STAGE2_LOAD_OFFSET

.loop:
	mov     ax, [di]
	mov     si, [di + 2]
	add     di, 4
	mov     cl, [di]
	inc     di

	cmp     cl, 0
	je      .read_finish

	call    [_read_lba]

	push    di
	xor     ch, ch
	shl     cx, 5
	mov     di, es
	add     di, cx
	mov     es, di
	pop     di

	jmp     .loop
.read_finish:

	pushf
	pop     ax
	or      ax, 0xF000							; force bits 12-15 high
	push    ax
	popf
	pushf
	pop     ax
	and     ax, 0xF000
	cmp     ax, 0xF000
	je      .not_386							; bits stuck -> 286 or older
	mov     byte [cpu_tier], 1					; 386+
	jmp     .done
.not_386:
	mov     byte [cpu_tier], 0					; pre-386
global .done
.done:

	mov     bl, [cpu_tier]
	mov     dl, [ebr_drive_number]
	mov     si, [partition_offset]
	xor     di, di
	mov     cx, [partition_entry_index]

	mov     ax, STAGE2_LOAD_SEGMENT
	mov     es, ax
	mov     ds, ax
.Jump:
	jmp     STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

section .text

section .rodata

STAGE2_LOAD_SEGMENT equ     0x0
STAGE2_LOAD_OFFSET equ     buffer

PARTITION_ENTRY_SEGMENT equ     0
PARTITION_ENTRY_OFFSET equ     500

section .data
global stage2_location
stage2_location:
	times   15 db 0

global pre_386_location
pre_386_location:
	times   15 db 0

partition_offset:
	dw      0
partition_entry_index:
	dw      0
cpu_tier:
	db      0

section .bss
buffer:
	resb    512