; ========== boot16.s
section .text16
BITS 16
start16:
    cli
    jmp real_mode_start16

; ===========   boot16_2.s
; ===================== .data16/.data32 =====================
section .data16
BITS 16

gdt_start:
    ; Null descriptor
    dq 0x0000000000000000

    ; 32-bit code segment (base=0, limit=4GB, present, ring 0, code)
    dq 0x00CF9A000000FFFF

    ; 32-bit data segment (base=0, limit=4GB, present, ring 0, data)
    dq 0x00CF92000000FFFF

    ; 64-bit code segment (base=0, limit=0, long mode)
    dq 0x00209A0000000000

    ; 64-bit data segment (base=0, limit=0, flat)
    dq 0x0000920000000000
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; 16-bit GDT limit
    dd gdt_start                  ; 32-bit base address for pr


section .text16
BITS 16
real_mode_start16:
    ; setup stack, GDT
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE32:pm32_start

; ================  boot32.s
section .bss32
BITS 32
align 16
stack_bottom:
	resb 16384 ; 16 KiB
stack_top:




section .text32
[BITS 32]
pm32_start:
    ; set up data segments, page tables
    mov ax, DATA32
    mov ds, ax
    mov ss, ax
    mov esp, stack_top
    ; enable PAE, long mode
    ; ...

; boot64.s
section .text64
[BITS 64]
long_mode_entry:
    mov rdi, 0xB8000
    lea rsi, [rel msg]
.loop:
    lodsb
    or al, al
    jz .done
    mov [rdi], ax
    add rdi, 2
    jmp .loop
.done:
    hlt

section .data64
msg db 'Hello World',0

