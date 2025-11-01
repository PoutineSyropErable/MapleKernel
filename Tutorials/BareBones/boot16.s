; =====================================================
; boot16_flat.s - 16-bit real mode entry (flat binary)
; =====================================================
BITS 16
ORG 0x7000               ; load address in memory

; -----------------------------
; 16-bit stack (4 KB)
; -----------------------------
boot16_stack_bottom:
    TIMES 4096 DB 0
boot16_stack_top:

; -----------------------------
; GDT (3 entries)
; -----------------------------
gdt_start:
    DQ 0x0000000000000000       ; null descriptor
    DQ 0x0000000000000000       ; extra null
    DQ 0x00CF9A000000FFFF       ; 32-bit code segment
    DQ 0x00CF93000000FFFF       ; 32-bit data segment

gdt_descriptor:
    DW gdt_end - gdt_start - 1  ; limit
    DD gdt_start                ; base
gdt_end:

; -----------------------------
; Entry point
; -----------------------------
boot16_start:

    cli                         ; disable interrupts

    ; -----------------------------
    ; Set up 16-bit stack
    ; -----------------------------
    mov eax, boot16_stack_bottom
    shr eax, 4                  ; convert byte address to segment
    mov ss, ax
    mov sp, boot16_stack_top - boot16_stack_bottom

    ; -----------------------------
    ; Load GDT
    ; -----------------------------
    lgdt [gdt_descriptor]

    ; -----------------------------
    ; Enable protected mode
    ; -----------------------------
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; -----------------------------
    ; Far jump to 32-bit kernel
    ; -----------------------------
	%define boot32_start 0x20000
    jmp 0x10:boot32_start       ; CS selector = 0x10

halt_loop16:
    hlt
    jmp halt_loop16

