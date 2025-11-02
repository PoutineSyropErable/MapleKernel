; add16_wrapper16.s
BITS 16


global add1616_start 
extern resume32 
extern args16_start

section .text.add1616

add1616_start:
    mov bx, 0xFAC2
	; To find it easier in the binary

    ; --- Switch back to protected mode ---
    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    mov esp, [args16_start]
    mov ax, 0x18
    mov ss, ax
    mov ds, ax

    jmp far 0x10:resume32

halt_loop3:
; This should never be reached
    hlt
    jmp halt_loop3

