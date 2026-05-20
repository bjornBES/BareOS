
.intel_syntax

struc Line 
    .line_num resd 1  ; int line_no;    offset 0
    .text resq 1 ; char *text;          offset 4
    .next resq 1 ; struct Line *next;   offset 12
endstruc


struc Token
    .token_type: resd 1
    .text: resq 1
endstruc


