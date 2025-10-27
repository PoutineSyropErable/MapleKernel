BITS 32

global call_add16
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


	mov eax, 0xdeadbeef
	mov [args16_start], esp      ; 
    cli

    ; --- Switch to 16-bit mode (cr0.PE = 0)---
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax

	; sets up the data segment 
	mov ax, 0
	mov ds, ax

    ; Set up 16-bit stack
    mov ax, word stack16_start
    shr ax, 4             ; segment = address >> 4
    mov ss, ax
    mov sp, 0x4000        ; top of 16-bit stack
	; 1024  = 0x0400 = 1.00 KB
	; 2048  = 0x0800 = 2.00 KB
	; 4096  = 0x1000 = 4.00 KB
	; 8192  = 0x2000 = 8.00 KB
	; 16384 = 0x4000 = 16.00 KB



    ; Far jump to 16-bit wrapper
	; Code is in the first 64kb, 
	jmp far 00:add1616_start




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
	mov eax, 15

    ret

