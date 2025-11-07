;add16_wrapper32.s
BITS 32

global call_add16
global resume32
extern add1616_start
extern protected_16

extern stack16_start
extern stack16_end
extern args16_start
extern GDT16_DESCRIPTOR 

section .data.misc32.1

GDT16_DESCRIPTOR:
	dw GDT_END - GDT_START - 1 ;limit/size
	dd GDT_START ; base

GDT_START: 
	dq 0x0 
	dq 0x0
	dq 0x00009A000000FFFF ; code 
	dq 0x000093000000FFFF ; data
GDT_END:

section .text.add1632

call_add16:
	mov eax, 0xdeadfac1

    ; Save 32-bit registers and flags
    pushad
    pushfd
    push ds
    push es
    push fs
    push gs

	; rdi = arg1 
	; rsi = arg2
	; Save the stack pointer in the first 1mb (first 64kb in fact)
	; So its accessible in 16 bit, and can be restored on the way back to 32 bit
	mov [args16_start], esp      ; 
	mov [args16_start +4], dx      ; 
	mov [args16_start +6], cx      ; 
	sgdt [args16_start + 10]

	mov esp, 0 ; in case i can't change esp in 16 bit mode later. Don't want the high bit to fuck us over
	mov ebp, 0 ; in case i can't change esp in 16 bit mode later. Don't want the high bit to fuck us over


    cli

	lgdt [GDT16_DESCRIPTOR]
	jmp far 0x10:protected_16




halt_loop: 
	hlt 
	jmp halt_loop
;
; reset:
; 	cli                     ; disable interrupts
; 	xor eax, eax
; 	lidt [eax]              ; load IDT base = 0, limit = 0 (invalid)
; 	int 3                   ; trigger interrupt -> #GP -> #DF -> triple fault





section .text.resume32
; resume32 will be called by the 16-bit code when done
resume32:
    ; Restore segment registers


    mov esp, [args16_start]
	mov ax, 0x18 
	mov ss, ax

    pop gs
    pop fs
    pop es
    pop ds

    ; Restore general-purpose registers and flags
    popfd
    popad


    ; Retrieve result
    movzx eax, word [args16_start + 8]
	; mov eax, 15

    ret


halt_loop2: 
	hlt 
	jmp halt_loop2

