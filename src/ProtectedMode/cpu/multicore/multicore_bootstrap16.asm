BITS 16

extern core_has_booted 
extern master_tells_core_to_start
extern new_gdt
extern core_bootstrap32

extern __new_gdt_start
extern __new_gdt_end


%define MAX_CPU_COUNT 12 ; This must be > (Actual cpu count, so off by 1 error)
%define NEW_GDT_LIMIT (4 + 2*MAX_CPU_COUNT) - 1

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


	mov ax, MAX_CPU_COUNT 
	sub ax, 1
	imul ax, 2
	add ax, 3
	imul ax, 8 ; (4 + 2*MAX_CPU_COUNT) - 1
	; mov ax, NEW_GDT_LIMIT 
	mov [gdt_descriptor], ax  ; limit
	lgdt [gdt_descriptor]


    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp far 0x10:core_bootstrap32



