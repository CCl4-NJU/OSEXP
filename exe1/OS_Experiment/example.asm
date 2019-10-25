    org 0x07c0

_start:
	mov eax, msg
	call strlen

	mov edx, eax
	mov ecx, msg
	mov ebx, 1
	mov eax, 4
	int 80h

	mov ebx, 0
	mov eax, 1
	int 80h

strlen:
	push ebx
	mov ebx, eax

nextchar:
	cmp byte[eax], 0
	jz finished
	inc eax
	jmp nextchar

finished:
	sub eax, ebx
	pop ebx
	ret

msg: db "Hello, brave new world!", 0Ah
times   510-($-$$)  db 0
dw  0xaa55
