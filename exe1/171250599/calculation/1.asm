section .bss
	x: resb 22
	y: resb 22
	reversex: resb 22
        reversey: resb 22
        result: resb 22
        reverseres: resb 22

section .data
;        x db "99999999999999999999",0h
;        y db "99999999999999999999",0h
        carry db "1",0h
        mulres TIMES 41 db 0
        revmulres TIMES 41 db 0
        LF db 0ah, 0h
        LFlen equ $-LF

section .text
global _start

_start:
    mov rbp, rsp; for correct debugging
; read x
	mov eax, 3
	mov ebx, 0
	mov ecx, x
	mov edx, 21
	int 80h
; read y
	mov eax, 3
	mov ebx, 0
	mov ecx, y
	mov edx, 21
	int 80h

	mov rax, x
	call slen
	mov r12, rax
	dec r12
        mov WORD [x+r12], 0h
; r12 store the bit length of x
	mov rax, y
	call slen
	mov r13, rax
	dec r13
        mov WORD [y+r13], 0h
;        mov rax, y
 ;       call slen
; r13 store the bit length of y

; use r10 and r11 to store x and y
; use r12 and r13 to store length
; r10 store the one with longer length, default x
; r11 store the one with shorter length, default y
; r12 store the longer length, default x-length
; r13 store the shorter length, default y-length
	mov r10, x
	mov r11, y
; now value in r10-r13 won't change
; use functions to show result of add and mul

        call showAddition

        mov rax, 4
        mov rbx, 1
        mov rcx, LF
        mov rdx, LFlen
        int 80h

; now reversx and reversey are all calculated
        call showMultiplication

	call quit
;-----------------------------------------------------------------------------
showAddition:
        push rsi
        push rax
        push rbx
        push rcx
        push rdx

; reverse x-------------------------------------------------------
	xor rcx, rcx
	mov rdx, r12

reversexPushLoop:
        mov ax, WORD [r10+rcx]
	push ax

	inc rcx
	dec rdx
	cmp rdx, 0
	jg reversexPushLoop

	xor rcx, rcx
	mov rdx, r12

reversexPopLoop:
        pop WORD [reversex+rcx]
	inc rcx
	dec rdx
	cmp rdx, 0
	jg reversexPopLoop
        mov WORD [reversex+rcx], 0h

; reverse y---------------------------------------------------------
        xor rcx, rcx
	mov rdx, r13

reverseyPushLoop:
        mov ax, [r11+rcx]
	push ax
	inc rcx
	dec rdx
	cmp rdx, 0
	jg reverseyPushLoop

	xor rcx, rcx
	mov rdx, r13

reverseyPopLoop:
        pop WORD [reversey+rcx]
	inc rcx
	dec rdx
	cmp rdx, 0
	jg reverseyPopLoop
        mov WORD [reversey+rcx], 0h
; reverse x and y finished---------------------------------------
 
; put smaller lens in rcx and 0 in rdx
; r15 represent the loop times and rcx is the offset
; r14 is used to represent carry, which is 0 when start
        mov rcx, 0
        mov r15, r13
        mov r14, 0
        mov r10, reversex
        mov r11, reversey
        
additionLoop:
        mov al, [r10+rcx]
        mov bl, [r11+rcx]
        sub al, 48
        sub bl, 48
        add bl, al
        add rbx, r14
        xor al, al
        xor rdx, rdx
        mov al, bl
        
        mov bx, 10
        div bx
; quotient in rax, remainder in rdx
; change val in rdx back to ascii
        add dx, 48
        mov r14, rax
        mov WORD [result+rcx], dx
        dec r15
        inc rcx
        cmp r15, 0
        jg additionLoop
        
; unless len_x = len_y, there are still bits left to be added
        cmp r12, r13
        je finishAddition
finalLoop:
; mimic the step above, instead r9 now stores carry previous
        mov al, [reversex+rcx]
        mov rbx, r14
        sub al, 48
        add rbx, rax
        xor rax, rax
        mov rax, rbx
        mov rbx, 10
        xor rdx, rdx
        div rbx
        add rdx, 48
        mov r14, rax
        mov WORD [result+rcx], dx
        inc rcx
        cmp r12, rcx
        jg finalLoop
 
finishAddition:       
; reverse res-------------------------------------------------------
	xor rcx, rcx
	mov rdx, r12

reverseresPushLoop:
        mov ax, [result+rcx]
	push ax
	inc rcx
	dec rdx
	cmp rdx, 0
	jg reverseresPushLoop

	xor rcx, rcx
	mov rdx, r12

reverseresPopLoop:
        pop WORD [reverseres+rcx]
	inc rcx
	dec rdx
	cmp rdx, 0
	jg reverseresPopLoop
        mov WORD [reverseres+rcx], 0h
       
; before print, check highest carry
        cmp r14, 0
        jg outputCarry               

outputRest:       
       mov rax, reverseres
       call sprintLF
       
	pop rdx
        pop rcx
        pop rbx
        pop rax
        pop rsi

	ret

outputCarry:
        mov rax, carry
        call sprint
        jmp outputRest

;-----------------------------------------------------------------------------
showMultiplication:

        push rsi
        push rax
        push rbx
        push rcx
        push rdx

        mov rax, x
        call slen
        mov r10, rax
        
        mov rax, y
        call slen
        mov r11, rax
        
; r10 stores x_len
; r11 stores y_len
; both are used for counters
; r10 is bigger
        xor r8, r8
        xor r12, r12
        xor r13, r13
        xor rax, rax
        xor rbx, rbx
        xor rcx, rcx
        xor rdx, rdx
; r12 is index for reverseX
; r13 is index for reverseY
outloop:
        xor rax, rax
        mov al, BYTE[reversex+r12]
        sub al, 48
inloop:
        mov r8, rax
; temporary save rax in r8
        xor rbx, rbx
        xor rdx, rdx
        mov bl, BYTE[reversey+r13]
        sub bl, 48
        mov cl, BYTE[revmulres+r12+r13]
        mul rbx
        add rax, rcx
; now rax stores the product for two bits, also adding previous carry
; next figure out the carry and store the remainder in revmulres
; which stands for reverse-multiplication-result
        mov rbx, 10
        div rbx
; currently, rax stores the carry and rdx stores the remainder
; move the remainder into current res-bit and add carry to next res-bit
        mov [revmulres+r12+r13], dl
        inc r13
        mov cl, BYTE [revmulres+r12+r13]
        add rcx, rax
        mov [revmulres+r12+r13], cl
        dec r13
; after processing bit storing, check loop condition
; also restore rax
        mov rax, r8
        inc r13
        cmp r13, r11
        jl inloop
        xor r13, r13
        inc r12
        cmp r12, r10
        jl outloop
        
addEnd:
        mov BYTE [revmulres+r10+r11], 0h
        
; now change revmulres back to mulres--multiplication-result
        xor r12, r12
        xor r13, r13
        add r11, r10
        mov BYTE [mulres+r11], 0h
        dec r11
; now r11 is the total loop times -> len(x)+len(y)-1
; r11 is also the index for mulres
; use r12 to index revmulres
mulresloop:
        mov cl, BYTE [revmulres+r12]
        add cl, 48
        mov [mulres+r11], cl
        dec r11
        inc r12
        cmp r11, 0
        jge mulresloop
        
; now output mulres
	mov cl, [mulres]
	sub cl, 48
	cmp cl, 0
	jg outputProduct1
	mov rsi, mulres
	inc rsi
	mov rax, rsi
	call sprint
	jmp backToStart

outputProduct1:
        mov rax, mulres
        call sprint
        
backToStart:
        pop rdx
        pop rcx
        pop rbx
        pop rax
        pop rsi
        Ret

utils.asm源代码 - 工具类，提供简单的输入输出等功能

; int atoi(Integer number)
; Ascii to integer function (atoi)
atoi:
	push rbx
	push rcx
	push rdx
	push rsi
	mov rsi, rax
	mov rax, 0
	mov rcx, 0

.multiplyLoop:
	xor rbx, rbx
	mov bl, [esi+ecx]
	cmp bl, 48
	jl .finished
	cmp bl, 57
	jg .finished

	sub bl, 48
	add rax, rbx
	mov rbx, 10
	mul rbx
	inc rcx
	jmp .multiplyLoop

.finished:
	mov rbx, 10
	div rbx
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	ret


; void iprint(Integer number)
; INteger printing function (itoa)
iprint:
	push rax
	push rcx
	push rdx
	push rsi
	mov rcx, 0

divideLoop:
	inc rcx
	mov rdx, 0
	mov rsi, 10
	idiv rsi
	add rdx, 48
	push rdx
	cmp rax, 0
	jnz divideLoop

printLoop:
	dec rcx
	mov rax, rsp
	call sprint
	pop rax
	cmp rcx, 0
	jnz printLoop

	pop rsi
	pop rdx
	pop rcx
	pop rax
	ret

; void iprintLF(Integer number)
; Integer printing function with linefeed (itoa)
iprintLF:
	call iprint

	push rax
	mov rax, 0Ah
	push rax
	mov rax, rsp
	call sprint
	pop rax
	pop rax
	ret

; int slen(String message)
; String length calculation function
slen:
	push rbx
	mov rbx, rax

nextchar:
	cmp byte [rax], 0
	jz finished
	inc rax
	jmp nextchar

finished:
	sub rax, rbx
	pop rbx
	ret

; void sprint(String message)
; String printing function
sprint:
	push rdx
	push rcx
	push rbx
	push rax
	call slen

	mov rdx, rax
	pop rax

	mov rcx, rax
	mov rbx, 1
	mov rax, 4
	int 80h

	pop rbx
	pop rcx
	pop rdx
	ret

; void sprintLF(String message)
; String printing with line feed function
sprintLF:
	call sprint

	push rax
	mov rax, 0Ah
	push rax
	mov rax, rsp
	call sprint
	pop rax
	pop rax
	ret


; void exit()
; Exit program and restore resources
quit:
	mov rbx, 0
	mov rax, 1
	int 80h
	ret

; int atoi(Integer number)
; Ascii to integer function (atoi)
atoi:
push rbx
push rcx
push rdx
push rsi
mov rsi, rax
mov rax, 0
mov rcx, 0

.multiplyLoop:
xor rbx, rbx
mov bl, [esi+ecx]
cmp bl, 48
jl .finished
cmp bl, 57
jg .finished

sub bl, 48
add rax, rbx
mov rbx, 10
mul rbx
inc rcx
jmp .multiplyLoop

.finished:
mov rbx, 10
div rbx
pop rsi
pop rdx
pop rcx
pop rbx
ret


; void iprint(Integer number)
; INteger printing function (itoa)
iprint:
push rax
push rcx
push rdx
push rsi
mov rcx, 0

divideLoop:
inc rcx
mov rdx, 0
mov rsi, 10
idiv rsi
add rdx, 48
push rdx
cmp rax, 0
jnz divideLoop

printLoop:
dec rcx
mov rax, rsp
call sprint
pop rax
cmp rcx, 0
jnz printLoop

pop rsi
pop rdx
pop rcx
pop rax
ret

; void iprintLF(Integer number)
; Integer printing function with linefeed (itoa)
iprintLF:
call iprint

push rax
mov rax, 0Ah
push rax
mov rax, rsp
call sprint
pop rax
pop rax
ret

; int slen(String message)
; String length calculation function
slen:
push rbx
mov rbx, rax

nextchar:
cmp byte [rax], 0
jz finished
inc rax
jmp nextchar

finished:
sub rax, rbx
pop rbx
ret

; void sprint(String message)
; String printing function
sprint:
push rdx
push rcx
push rbx
push rax
call slen

mov rdx, rax
pop rax

mov rcx, rax
mov rbx, 1
mov rax, 4
int 80h

pop rbx
pop rcx
pop rdx
ret

; void sprintLF(String message)
; String printing with line feed function
sprintLF:
call sprint

push rax
mov rax, 0Ah
push rax
mov rax, rsp
call sprint
pop rax
pop rax
ret

; void exit()
; Exit program and restore resources
quit:
mov rbx, 0
mov rax, 1
int 80h
ret