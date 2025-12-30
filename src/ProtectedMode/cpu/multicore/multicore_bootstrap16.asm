BITS 16

extern core_has_booted 
extern master_tells_core_to_start
extern new_gdt
extern core_bootstrap32

extern __new_gdt_start
extern __new_gdt_end


%include "core_count_read_only.inc"

section .bss.multicore_bootstrap16
gdt_descriptor:
    dw 0      ; limit
    dd 0      ; base
section .data.multicore_bootstrap16

section .rodata.multicore_bootstrap16


section .text.multicore_bootstrap16
global core_bootstrap
core_bootstrap: 
	mov bx, new_gdt  ; base
	mov [gdt_descriptor+2], bx


	mov ax, MAX_CORE_COUNT 
	imul ax, 2
	add ax, 4
	imul ax, 8 ; (4 + 2*MAX_CORE_COUNT)
	; mov ax, NEW_GDT_LIMIT 
	mov [gdt_descriptor], ax  ; limit
	lgdt [gdt_descriptor]


    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp far 0x10:core_bootstrap32



