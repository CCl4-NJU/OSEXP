%include "io.inc"
%include "utils.asm"
section .data
    msg db "HW",0

section .text
global CMAIN
CMAIN:
    mov msg, eax
    call sprint
    ;write your code here
    xor eax, eax
    ret