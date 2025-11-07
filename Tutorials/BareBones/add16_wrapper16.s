; add16_wrapper16.s
BITS 16

extern resume32 
extern args16_start
extern stack16_start
extern add16

global add1616_start 
global protected_16

section .text.add1616
protected_16: 
	mov ax, 0x18 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax


    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    ; Far jump to 16-bit wrapper
	jmp far 00:add1616_start

add1616_start:
	; setup 16 bit data segment
	mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax  
    mov gs, ax

	mov ax, stack16_start
	shr ax, 4
	mov ss, ax
    mov ss, ax
    mov esp, 0x4000
    
    ; Set up real mode stack HERE


	mov ax, [args16_start +4]     ; 
	mov bx, [args16_start +6]     ; 
	push ax 
	push bx
	call add16
	mov [args16_start+8], ax
	pop ax 
	pop bx



    ; --- Switch back to protected mode ---
	lgdt [args16_start + 10]
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp far 0x10:resume32

halt_loop3:
; This should never be reached
    hlt
    jmp halt_loop3




; ============= Byte <-> Hex Table ============
; 1024  = 0x0400 = 1.00 KB
; 2048  = 0x0800 = 2.00 KB
; 4096  = 0x1000 = 4.00 KB
; 8192  = 0x2000 = 8.00 KB
; 16384 = 0x4000 = 16.00 KB
; 32768 = 0x8000 = 32.00 KB
; 65535 = 0xFFFF = 64.00 KB -1 Byte
; 65536 =0x10000 = 64.00 KB


; 0x20000 = 128.00 KB
; 0x40000 = 256.00 KB
; 0x80000 = 512.00 KB
; 0xFFFFF = 1MB - 1B
; 0x100000 = 1024.00 KB
