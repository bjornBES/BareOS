
section .text

[bits 64]

entry_64:
    mov rax, 0

.enter_64:
    mov rax, cr0
    or rax, 0x00
    mov cr0, rax