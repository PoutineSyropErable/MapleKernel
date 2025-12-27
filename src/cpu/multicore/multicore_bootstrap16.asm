BITS 16

extern core_has_booted 
extern master_tells_core_to_start
extern new_gdt

extern __new_gdt_start
extern __new_gdt_end


%define MAX_CPU_COUNT 8

section .bss.multicore_bootstrap16
section .data.multicore_bootstrap16
section .rodata.multicore_bootstrap16


section .text.multicore_bootstrap16
global core_bootstrap
core_bootstrap: 
	mov ax, 1
    ; mov ax, __new_gdt_end
    ; sub ax, __new_gdt_start
    ; dec ax                ; limit = size-1


