; add16_wrapper16.s
BITS 16
ORG 0        ; where the linker/script loads this in memory

; declare “externs” as constants
args16_start equ 0xB020
resume32     equ 0xB0B0

add1616_start:
    mov bx, 0xFAC2

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
    hlt
    jmp halt_loop3

