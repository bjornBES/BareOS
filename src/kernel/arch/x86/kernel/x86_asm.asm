;
; File: x86_asm.asm
; File Created: 20 Jan 2026
; Author: BjornBEs
; -----
; Last Modified: 25 Mar 2026
; Modified By: BjornBEs
; -----
;

[bits 64]

global arch_panic
arch_panic:
    cli
    hlt