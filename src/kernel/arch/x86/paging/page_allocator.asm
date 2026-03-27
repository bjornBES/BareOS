;
; File: page_allocator.asm
; File Created: 26 Feb 2026
; Author: BjornBEs
; -----
; Last Modified: 26 Feb 2026
; Modified By: BjornBEs
; -----
;

%if 0

[bits 64]
    
; definitions
_4KB            equ 4096


PAGE_SIZE       equ _4KB
MAX_PAGES       equ 65536

section         .data

section         .bss
; uint8 page_bitmap[8192]
global page_bitmap
page_bitmap:    resb 8192

; uint32* p_table
align           _4KB
global          p_table
p_table:        resd 1
    
; uint32* page_directory
global          page_directory
page_directory: resd 1

section         .text
; Note: all functions will use the cdecl calling conversion form ASMCALL
; #define ASMCALL __attribute__((cdecl)) src/libs/core/Defs.h

;
; signature:
;   ASMCALL void init_paging(void* page_table);
;
global          init_paging
init_paging:
    push        ebp
    mov         ebp,            esp
    
    mov         ebx,            [ebp + 8]       ;
    mov         eax,            [ebx]
    mov         [p_table],      eax             ; move the page table into the data section
    push dword  0x300
    call        mark_page_used                  ; marking the kernel entry used

    ret
; signature:
;   ASMCALL void *get_physaddr(void *virtualaddr);
;
; return: Returns the physical address based on the virtual address
;
; implementation from src/Bootloader/stage2/paging/paging.c
;
global          get_physaddr
get_physaddr:
    push        ebp
    mov         ebp,            esp

    push        ebx
    push        edx
    push        esi

    ; unsigned long pt_index = (unsigned long)virtualaddr >> 12 & 0x03FF;
    mov         eax,            [ebp + 8]       ;
    shr         eax,            12              ; eax >> 12
    and         eax,            0x03FF          ; eax & 0x03FF
    mov         esi,            eax             ;
    
    ; unsigned long pd_index = (unsigned long)virtualaddr >> 22;
    mov         eax,            [ebp + 8]       ;
    shr         eax,            22              ; eax >> 22
    mov         ebx,            eax             ; save it to pd_index

    ; unsigned long *pt = (p_table) + (0x400 * pd_index);
    shl         ebx,            10              ; saved eax (pd_index) for this pd_index * 0x400
                                                ; i fucking hate mul so we using shl
    add         ebx,            p_table         ; using ebx as the base for p_table

    ; return (pt[pt_index] & ~0xFFF) + (virtualaddr & 0xFFF);
                                                ; pt[pt_index] & ~0xFFF
    mov         edx,            [ebx + esi*4]   ; pt[pt_index] and each entry is 4 bytes
    and         edx,            0xFFFFF000      ; yes

                                                ; virtualaddr & 0xFFF
    mov         eax,            [ebp + 8]       ;
    and         eax,            0xFFF           ; offset in page

    add         eax,            edx             ; using eax as the first operand so we can return

    pop         esi
    pop         edx
    pop         ebx

    pop         ebp
    ret

;
; signature:
;   ASMCALL void *alloc_page();
;
; return: Returns the physical address of the page
;
global alloc_page
alloc_page:
    push        ebp
    mov         ebp,            esp


;
; signature:
;   ASMCALL void free_page(void* page);
;
global free_page
free_page:


;
; HELPER FUNCTIONS
;

;
; signature:
;   ASMCALL void mark_page_used(uint32 page_index)
;
global mark_page_used
mark_page_used:
    push        ebp
    mov         ebp,            esp

    push        eax
    push        ebx
    push        ecx
    push        edx
    push        edi
    push        esi

    xor         edx,            edx
    mov         eax,            [ebp + 8]       ; Divide takes edx:eax / operand1
    mov         ecx,            8

    div         ecx                             ; eax / 8 Results are in EAX := Quotient, EDX := Remainder
    mov         esi,            eax             ; move the quotient to the source index
    mov         ebx,            page_bitmap     ; using ebx as the base register for page_bitmap
    push        eax                             ; save eax
    push        ebx
    call        get_physaddr                    ; get the phys address of page_bitmap
    mov         ebx,            eax             ; eax is the result
    pop         eax

    mov         cl,             dl
    mov         edx,            1
    shr         edx,            cl              ; 1 << remainder (page_index % 8)
    
    lea         edi,            [ebx + esi]
    mov         esi,            edi
    lodsd                                       ; load eax from [esi]
    or dword    eax,            edx             ; set the bit
    stosw                                       ; store eax to [edi]
    
    pop         esi
    pop         edi
    pop         edx
    pop         ecx
    pop         ebx
    pop         eax

    pop         ebp
    ret

;
; signature:
;   ASMCALL void mark_page_free(uint32 page_index)
;
global mark_page_free
mark_page_free:
    push        ebp
    mov         ebp,            esp

    push        eax
    push        ebx
    push        ecx
    push        edx
    push        edi
    push        esi

    xor         edx,            edx
    mov         eax,            [ebp + 8]       ; Divide takes edx:eax / operand1
    mov         ecx,            8

    div         ecx                             ; eax / 8 Results are in EAX := Quotient, EDX := Remainder
    mov         esi,            eax             ; move the quotient to the source index
    mov         ebx,            page_bitmap     ; using ebx as the base register for page_bitmap

    mov         cl,             dl
    mov         edx,            1
    shr         edx,            cl              ; 1 << remainder (page_index % 8)
    neg         edx                             ; ~ (1 << (page_index % 8))

    lea         edi,            [ebx + esi * 4]
    mov         esi,            edi
    lodsd                                       ; load eax from [esi]
    and dword   eax,            edx             ; zero the bit
    stosw                                       ; store eax to [edi]
    
    pop         esi
    pop         edi
    pop         edx
    pop         ecx
    pop         ebx
    pop         eax

    pop         ebp
    ret

;
; signature:
;   ASMCALL int find_free_page();
;
; return: Returns a free page index returns -1 if there was no more pages
;
global find_free_page
find_free_page:
    push        ebp
    mov         ebp,            esp

    push        ebx
    push        ecx
    push        edx
    push        edi
    push        esi

    mov         ecx,            MAX_PAGES + 1   ; set the loop count
    xor         esi,            esi             ; using esi as the index
    mov         ebx,            page_bitmap     ; using ebx as the base address for page_bitmap
.start_loop:

    ; page_bitmap[i / 8] & (1 << (i % 8))
    xor         edx,            edx             ; zero edx
    mov         eax,            esi             ; Divide takes EAX / operand1
    mov         ebx,            8

    div         ebx                             ; EAX / 8 Results are in EAX := Quotient, EDX := Remainder
    mov         edi,            eax             ; move the quotient to EDI

    push        ecx                             ; save count
    mov         cl,             dl              ; get the remainder into cl
    mov         dx,             1
    shr         dx,             cl              ; 1 << remainder (page_index % 8)
    pop         ecx                             ; restore count

    push        esi
    lea         esi,            [ebx + edi]
    lodsb                                       ; load al from [esi]
    pop         esi

    ; if(!eax)
    test        al,             dl              ; page_bitmap[i / 8] & (1 << (i % 8))
    jnz         .got_page                       ; expr^ != 0

.continue:
    inc         esi                             ; inc index before loop
    loop        .start_loop                     ; ecx -= 1 & ecx != 0 jump
    mov         eax,            -1              ; fail code
    jmp         .exit                           ; failed

.got_page:
    mov         eax,            esi

.exit:
    pop         esi
    pop         edi
    pop         edx
    pop         ecx
    pop         ebx
    pop         ebp
    ret
%endif