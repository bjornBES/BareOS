;
; File: function_table.inc
; File Created: 13 Aug 2026
; Author: BjornBEs
; -----
; Last Modified: 13 Aug 2026
; Modified By: BjornBEs
; -----
;

[bits 16]
section .jump_table
_read_lba:
    resw 1
_chs_to_lba:
    resw 1
_print:
    resw 1
_disk_reset:
    resw 1

; data
_have_extensions:
    resb 1
_drive_number:
    resb 1

    ; Disk Address Packet for INT 13h extension
extensions_dap:
    dap_size:           resb 1
    dap_reserved:       resb 1
    dap_count:          resw 1
    dap_offset:         resw 1
    dap_segment:        resw 1
    dap_lba:            resq 1