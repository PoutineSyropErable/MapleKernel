; -------------------------- boot32.s --------------------------
section .bss32
BITS 32
align 16
stack_bottom:
    resb 16384             ; 16 KiB stack
stack_top:

section .text32
BITS 32
pm32_start:
    mov ax, 0x10           ; DATA32 selector (from GDT)
    mov ds, ax
    mov ss, ax
    mov esp, stack_top

    ; enable PAE & long mode
    mov eax, cr4
    or eax, 0x20           ; set PAE
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100          ; set LME
    wrmsr

    mov eax, cr0
    or eax, 0x80000000     ; enable paging
    mov cr0, eax

    jmp LONGMODE:long_mode_entry

