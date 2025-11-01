; =====================================================
; boot32.s - 32-bit protected mode entry (called from boot16.s)
; =====================================================
BITS 32
GLOBAL boot32_start
EXTERN _start          ; kernel entry point in boot_intel.asm

section .text.boot32
boot32_start:

    ; -------------------------------------------------
    ; Set up data segments
    ; -------------------------------------------------
    mov ax, 0x18        ; data segment selector (gdt[3])
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; -------------------------------------------------
    ; Set up stack
    ; -------------------------------------------------
    mov esp, 0x9FC00    ; example 32-bit stack, adjust as needed

    ; -------------------------------------------------
    ; Jump to 32-bit kernel entry
    ; -------------------------------------------------
    call _start

halt_loop32:
    cli
    hlt
    jmp halt_loop32

