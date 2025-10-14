; ==============================================
; Combined 16-bit -> 32-bit -> 64-bit NASM boot
; ==============================================

; ------------------------------
; GDT and segment selectors
; ------------------------------
section .data16
BITS 16

gdt_start:
    dq 0x0000000000000000        ; Null descriptor
    dq 0x00CF9A000000FFFF        ; 32-bit code segment
    dq 0x00CF92000000FFFF        ; 32-bit data segment
    dq 0x00209A0000000000        ; 64-bit code segment
    dq 0x0000920000000000        ; 64-bit data segment
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Selectors
CODE32 equ 0x08
DATA32 equ 0x10
CODE64 equ 0x18
DATA64 equ 0x20

; ------------------------------
; 16-bit start / real mode code
; ------------------------------
section .text16
BITS 16
start16:
    cli
    jmp real_mode_start16

real_mode_start16:
    ; setup stack (optional)
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xFFFF

    ; load GDT
    lgdt [gdt_descriptor]

    ; enable protected mode
    mov eax, cr0
    or eax, 1           ; set PE
    mov cr0, eax

    ; far jump to 32-bit code
    jmp CODE32:pm32_start

; ------------------------------
; 32-bit protected mode code
; ------------------------------
section .bss32
BITS 32
align 16
stack_bottom:
    resb 16384          ; 16 KiB stack
stack_top:

section .text32
BITS 32
pm32_start:
    ; set up data segments
    mov ax, DATA32
    mov ds, ax
    mov ss, ax
    mov esp, stack_top

    ; enable PAE
    mov eax, cr4
    or eax, 0x20        ; set PAE
    mov cr4, eax

    ; enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100       ; set LME
    wrmsr

    ; enable paging
    mov eax, cr0
    or eax, 0x80000000  ; set PG
    mov cr0, eax

    ; far jump to 64-bit code
    jmp CODE64:long_mode_entry

; ------------------------------
; 64-bit long mode code
; ------------------------------
section .text64
BITS 64
long_mode_entry:
    mov rdi, 0xB8000         ; VGA text buffer
    lea rsi, [rel msg]       ; RIP-relative address of message

.loop64:
    lodsb                     ; load byte from [rsi] into al, increment rsi
    or al, al
    jz .done64
    mov [rdi], ax             ; write char + attribute
    add rdi, 2
    jmp .loop64

.done64:
    hlt

section .data64
BITS 64
msg db 'Hello World',0

