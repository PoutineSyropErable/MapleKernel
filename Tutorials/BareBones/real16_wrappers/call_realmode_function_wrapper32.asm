;call_realmode_function_wrapper32.s
BITS 32

global pm32_to_pm16
global resume32
extern pm16_to_real16

extern stack16_start
extern stack16_end
extern args16_start
extern GDT16_DESCRIPTOR 

%include "asm_constants.inc"   ; <-- include your header

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

section .text.pm32_to_pm16

; debugable? 
pm32_to_pm16:
    ; Save 32-bit registers and flags
    pushad
    pushfd
    push ds
    push es
    push fs
    push gs

	; Save the stack pointer in the first 1mb (first 64kb in fact)
	; So its accessible in 16 bit, and can be restored on the way back to 32 bit
	sgdt [args16_start + GDT_ROOT_OFFSET]
	mov [args16_start +  ESP_OFFSET], esp    ; 
	mov ax, ss
	mov [args16_start +  SS_OFFSET], ax    ; 




	mov esp, 0 ; in case i can't change esp in 16 bit mode later. Don't want the high bit to fuck us over
	mov ebp, 0 ; in case i can't change esp in 16 bit mode later. Don't want the high bit to fuck us over


    cli

	lgdt [GDT16_DESCRIPTOR]
	jmp far 0x10:pm16_to_real16




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


    mov esp, [args16_start + ESP_OFFSET]
	mov ax,  [args16_start + SS_OFFSET]
	mov ss, ax 
	mov ss, ax

    pop gs
    pop fs
    pop es
    pop ds

    ; Restore general-purpose registers and flags
    popfd
    popad


    ; Retrieve result
    movzx eax, word [args16_start + RET1_OFFSET]
    ; movzx edx, word [args16_start + RET2_OFFSET]

    ret


halt_loop2: 
	hlt 
	jmp halt_loop2

