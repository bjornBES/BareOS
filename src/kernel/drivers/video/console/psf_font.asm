;
; File: psf_font.asm
; File Created: 25 Oct 2026
; Author: BjornBEs
; -----
; Last Modified: 25 Oct 2026
; Modified By: BjornBEs
; -----
;

section .rodata

global _binary_font_psf_start
_binary_font_psf_start:
incbin 'drivers/video/console/default8x16.psf'
global _binary_font_psf_end
_binary_font_psf_end: