
[bits 64]

%include "types.asm"

;; int atoi (const char * str);
extern atoi

;; void *malloc(size_t size);
extern malloc

;; void *mmap(void *addr (NULL), size_t length, int prot, int flags, fd_t fd, off_t offset);
extern mmap

;; size_t write(fd_t fd, const void *buf, size_t len);
extern write

;; size_t read(fd_t fd, void *buf, size_t len);
extern read 


%define local(n) [rbp - (n*8)]

section .rodata
start_cursor: db "> "

section .text
global main
main:       ; scope 1
    push rbp
    mov rbp, rsp
    sub rsp, 1*8
    ; init

    xor rdi, rdi
    mov rsi, 1024*4
    mov rdx, 0x3            ;; PROT_READ ¦ PROT_WRITE
    mov r10, 0x20           ;; Anomymous
    mov r8, rdi
    mov r9, rdi
    call mmap
    mov local(1), rax
.loop:
    mov rdi, 1              ;; rdi = 1
    mov rsi, start_cursor
    mov rdx, 2
    call write

    dec rdi                 ;; rdi = 0 
    mov rsi, local(1)
    mov rdx, 1024*4
    call read

    mov rdi, rsi 
    call atoi
    
    cmp rax, 0
    jg .str_line
    call execute_line
    jmp .loop
.str_line:
    mov rsi, rdi
    mov rdi, rax
    call store_line
    jmp .loop

store_line: ; scope 1
    push rbp
    mov rbp, rsp
    push r12          ; save caller's r12
    push r13          ; save caller's r13
    sub rsp, 8*2            ;; 8*n where n is the number of local variables

    mov rax, [store_head]
    mov local(1), 0     ; Line *prev
    mov local(2), rax   ; Line *cur
    mov r12, rdi   ; int n
    mov r13, rsi   ; char *text


.loop: 
    ; while cur != null && cur->line_no < n:
    mov rax, local(2)
    cmp rax, 0
    je .loop_exit
    mov edx, [rax + Line.line_num]
    cmp edx, r12d
    jge .loop_exit
.loop_body:
    ;   prev = cur
    mov local(1), rax
    ;   cur = cur->next
    mov rdx, [rax + Line.next]
    mov local(2), rdx
    jmp .loop
.loop_exit:

    cmp rax, 0
    je .if0_next
    mov edx, [rax + Line.line_num]
    cmp edx, r12d
    jne .if0_next
    mov [rax + Line.text], r13
    jmp .store_line_ret
.if0_next:
    mov rsi, rax        ;; save cur        
    mov rdi, Line_size  ;; sizeof(Line)
    call malloc         ;; MALLOC
    mov rdi, rax        ;; rdi = result

    mov [rdi + Line.next], rsi
    mov [rdi + Line.line_num], r12d
    mov [rdi + Line.text], r13
    mov rax, local(1)

    cmp rax, 0
    jne .if1_else
    mov [store_head], rdi
    jmp .if1_next
.if1_else:
    mov [rax + Line.next], rdi 
.if1_next:
.store_line_ret:

    add rsp, 16
    pop r13
    pop r12

    pop rbp
    ret

execute_line:
    push rbp
    mov rsp, rbp
    sub rsp, 0


section .bss

; Line *store_head;
store_head: resq 1

