BOOTSEG equ 0x07c0
DISPLAYSEG equ 0xb800

_start:

	mov ax, BOOTSEG
	mov ds, ax
	
	mov ax, DISPLAYSEG
	mov es, ax
	
	mov cx, msglen
	mov si, message
	xor di, di

print_str:
	mov al, [si]
	mov [es:di], al
	inc si
	inc di
	mov byte[es:di], 0x07
	inc di
	loop print_str

	jmp near $

	message db "Loading System...", 13, 10
	msglen db $ - message

	times 510-($-$$) db 0
	dw 0xaa55
