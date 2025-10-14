[BITS 32]
pm32_start:
    ; set up stack, data segments
    mov ax, DATA32
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x9FFFF

    ; prepare page tables for 64-bit
    ; enable PAE, long mode
    mov eax, cr4
    or eax, 0x20      ; enable PAE
    mov cr4, eax

    mov eax, 0xC0000080
    rdmsr              ; enable LME (long mode)
    ; ... load PML4, enable paging

