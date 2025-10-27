BITS 32
section .text.add1632
global call_add16
extern add1616_start

extern stack16_start
extern stack16_end
extern args16_start

call_add16:
    ; Save 32-bit registers and flags
    pushad
    pushfd

    push ds
    push es
    push fs
    push gs

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
	; 1024  = 0x0400 = 1.00 KB
	; 2048  = 0x0800 = 1.00 KB
	; 4096  = 0x1000 = 1.00 KB
	; 8192  = 0x2000 = 1.00 KB
	; 16384 = 0x4000 = 1.00 KB


    ; Far jump to 16-bit wrapper
	jmp 00:add1616_start


; resume32 will be called by the 16-bit code when done
resume32:
    ; Switch back to 32-bit protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds

    ; Retrieve result
    movzx eax, word [args16_start]

    ; Restore general-purpose registers and flags
    popfd
    popad
    ret

