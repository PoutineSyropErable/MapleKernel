BITS 32

global call_add16
global resume32
extern add1616_start

extern stack16_start
extern stack16_end
extern args16_start

section .text.add1632

call_add16:

    ; Save 32-bit registers and flags
    pushad
    pushfd


    push ds
    push es
    push fs
    push gs


	mov eax, 0xdeadfac1

	; rdi = arg1 
	; rsi = arg2
	mov [args16_start], esp      ; 
    ; mov [args16_start+4], di     ; first argument 
    ; mov [args16_start+6], si     ; second argument
	; Putting the arguments on the arg16 section



    cli

    ; --- Switch to 16-bit mode ---

    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax

    ; Set up 16-bit stack
    mov ax, word stack16_start
    shr ax, 4             ; segment = address >> 4
    mov ss, ax
    mov sp, 0x4000        ; top of 16-bit stack

	mov ax, 0
	mov ds, ax
	; 1024  = 0x0400 = 1.00 KB
	; 2048  = 0x0800 = 1.00 KB
	; 4096  = 0x1000 = 1.00 KB
	; 8192  = 0x2000 = 1.00 KB
	; 16384 = 0x4000 = 1.00 KB


    ; Far jump to 16-bit wrapper

	jmp far 00:add1616_start


; halt_loop: 
; 	hlt 
; 	jmp halt_loop
;
; reset:
; 	cli                     ; disable interrupts
; 	xor eax, eax
; 	lidt [eax]              ; load IDT base = 0, limit = 0 (invalid)
; 	int 3                   ; trigger interrupt -> #GP -> #DF -> triple fault





; resume32 will be called by the 16-bit code when done
resume32:
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds

    ; Restore general-purpose registers and flags
    popfd
    popad


    ; Retrieve result
    ; movzx eax, word [args16_start + 8]
	mov eax, 15

    ret

