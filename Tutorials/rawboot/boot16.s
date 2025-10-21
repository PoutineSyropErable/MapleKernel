extern pm32_start
section .data16
BITS 16

gdt_start:
    dq 0x0000000000000000               ; Null descriptor
    dq 0x00CF9A000000FFFF               ; 32-bit code segment
    dq 0x00CF92000000FFFF               ; 32-bit data segment
    dq 0x00209A0000000000               ; 64-bit code segment
    dq 0x0000920000000000               ; 64-bit data segment
gdt_end:



section .text16
BITS 16
start16:
    cli
    jmp real_mode_start16


gdt_descriptor:
    dw gdt_end - gdt_start - 1          ; GDT limit
    dd gdt_start                        ; GDT base

section .text16
BITS 16
real_mode_start16:
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
	CODE32 equ 0x08
    jmp CODE32:pm32_start               ; far jump to 32-bit

