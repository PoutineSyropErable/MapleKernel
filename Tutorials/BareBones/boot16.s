; =====================================================
; boot16.s - 16-bit real mode entry
; =====================================================
BITS 16
GLOBAL boot16_start
EXTERN boot32_start   ; entry point in boot32.s

section .data.boot16
align 8
    gdt_start:
        dq 0x0000000000000000    ; null descriptor
        dq 0x0000000000000000    ; grub use an extra null
		dq 0x00CF9A000000FFFF   ; gdt[2] code segment
		dq 0x00CF93000000FFFF   ; gdt[3] data segment


    gdt_descriptor:
        dw gdt_end - gdt_start - 1  ; limit
        dd gdt_start                ; base

    gdt_end:

section .text.boot16
boot16_start:

    cli                 ; disable interrupts

    ; -------------------------------------------------
    ; Set up a 16-bit stack
    ; -------------------------------------------------
    mov ax, 0x7000      ; segment for 16-bit stack (below 1 MB)
    mov ss, ax
    mov sp, 0xFFF0      ; top of 16-bit stack

    ; -------------------------------------------------
    ; Setup GDT (like GRUB)
    ; -------------------------------------------------
    ; GDT structure: 3 entries (null, 16-bit, 32-bit)

    lgdt [gdt_descriptor]

    ; -------------------------------------------------
    ; Enable protected mode
    ; -------------------------------------------------
    mov eax, cr0
    or eax, 1           ; set PE bit
    mov cr0, eax

    ; Far jump to 32-bit code segment
    jmp 0x10:boot32_start    ; 0x08 = index of 32-bit code segment in GDT

halt_loop16:
    hlt
    jmp halt_loop16

