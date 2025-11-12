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

; Access bits
PRESENT        	equ 1 << 7
DPL2			equ 1 << 6 ; That's for DPL, we don't use it. 
DPL1 			equ 1 << 5 ; DPL1 | DPL2 = 2 + 1 = 3: DPL = 3. Since it's DPL0 
NOT_SYS        	equ 1 << 4
EXEC           	equ 1 << 3 ; If exec, then it's code. Otherwise it's data 
DC             	equ 1 << 2 ; Conforming for code, expand down for data. (We want non conforming, non expand down)
RW             	equ 1 << 1 ; Read for Code, Write for Data
ACCESSED     	equ 1 << 0

; Flags bits
GRAN_4K       	equ 1 << 7
SZ_32         	equ 1 << 6
LONG_MODE     	equ 1 << 5
AVAIL 			equ 1 << 5 ; That's for system available. Not using it right now
; sement limit high is <<[0,4]




GDT16_DESCRIPTOR:
	dw GDT16_END - GDT16_START - 1 ;limit/size
	dd GDT16_START ; base
GDT16_START: 
	.NULL: equ $ - GDT16_START
		dq 0x0
	.NULL2: equ $ - GDT16_START ; A grub detail
		dq 0x0
	.CODE: equ $ - GDT16_START
		.CODE.limit_low: dw 0xFFFF
		.CODE.base_low: dw 0
		.CODE.base_mid: db 0
		.CODE.access: db PRESENT | NOT_SYS | EXEC | RW
		.CODE.flags: db 0
		.CODE.base_high: db 0
	.DATA: equ $ - GDT16_START
		.DATA.limit_low: dw 0xFFFF
		.DATA.base_low: dw 0
		.DATA.base_mid: db 0
		.DATA.access: db PRESENT | NOT_SYS | RW
		.DATA.flags: db 0
		.DATA.base_high: db 0 
	; dq 0x00009A000000FFFF ; code 
	; dq 0x000093000000FFFF ; data
GDT16_END:


section .text.pm32_to_pm16


pm32_to_pm32_fixed: 
	; Write code to switch to a fixed CS, SS, DS PM32


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
	push cs
	pop ax 
	mov [args16_start +  CS_OFFSET], ax    ; 




	mov esp, 0 ; in case i can't change esp in 16 bit mode later. Don't want the high bit to fuck us over
	mov ebp, 0 ; in case i can't change esp in 16 bit mode later. Don't want the high bit to fuck us over


    cli

	; Note this is a direct jump to a CS=(Index=2, GDT, RPL=0)
	lgdt [GDT16_DESCRIPTOR]
	jmp far CODE_SEL:pm16_to_real16




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

