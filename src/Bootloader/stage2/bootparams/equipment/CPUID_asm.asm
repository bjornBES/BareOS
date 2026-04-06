[bits 32]

section .text

;
; void ASMCALL CPUID(uint32_t eax, uint32_t ecx, cpuid_regs* outregs);
;
global CPUID
CPUID:
    push ebp
    mov ebp, esp
    
    push ebx            ; only EBX must be preserved (cdecl)
    push esi

    mov eax, [ebp+0x8]  ; eax_value parameter
    mov ecx, [ebp+0xc]  ; ecx_value parameter
    cpuid               ; execute CPUID

    mov esi, [ebp+16]   ; outregs pointer

    ; Store CPUID results
    mov [esi + 0x0], eax
    mov [esi + 0x4], ebx
    mov [esi + 0x8], ecx
    mov [esi + 0xc], edx

    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret