
.intel_syntax

%include "types.asm"

section .text

%define TOK_NONE 0
%define TOK_EOF 1
%define TOK_NEWLINE 2

%define TOK_PRINT 3
 
;
; Token *tokenize(char *line)
;
global tokenize
tokenize:   ; scope 1


;
; Token *next_token(char *line)
;
next_token:
    push rbp
    mov rbp, rsp
    
    xor rcx, rcx
.skip_space:; scope 2
    mov al, [rdi + rcx]
    cmp al, 0
    je .next_token_ret
    cmp al, ' '
    jne .skip_space_next
    inc rcx
    jmp .skip_space
.skip_space_next:
    
    ; TODO: check a lot




