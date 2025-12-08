BITS 16

mov ax, 0x4F00
int 0x10
; On return:
; AX = 0x004F if supported
; ES:DI -> VBE info structure (optional)


section .data 

section .bss 

section .rodata



section .text

global detect_vesa_support 
detect_vesa_support: 
	mov ax, 0x4F00
	int 0x10
	; On return:
	; AX = 0x004F if supported
	; ES:DI -> VBE info structure (optional)
