; add16_wrapper16.s
BITS 16


global add1616_start 
extern resume32 
extern args16_start
extern stack16_start
extern add16

section .text.add1616

add1616_start:
	; setup 16 bit data segment
	mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax  
    mov gs, ax
    
    ; Set up real mode stack HERE
	mov ax, stack16_start
	shr ax, 4
	mov ss, ax
    mov ss, ax
    mov sp, 0x4000


	mov ax, [args16_start +4]       ; 
	mov bx, [args16_start +6]     ; 
	push ax 
	; push bx
	; call add16
	; mov [args16_start+8], ax
	pop ax 
	; pop bx


	; 1024  = 0x0400 = 1.00 KB
	; 2048  = 0x0800 = 1.00 KB
	; 4096  = 0x1000 = 1.00 KB
	; 8192  = 0x2000 = 1.00 KB
	; 16384 = 0x4000 = 1.00 KB

    ; --- Switch back to protected mode ---
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp far 0x10:resume32

halt_loop3:
; This should never be reached
    hlt
    jmp halt_loop3

