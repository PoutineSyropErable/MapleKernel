bits 16
start16:
    ; optional: set up stack
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xFFFF

    ; prepare GDT for protected mode
    lgdt [gdt_descriptor]

    ; enable A20 line (optional in QEMU, usually already on)

    ; enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE32:pm32_start  ; far jump loads CS

