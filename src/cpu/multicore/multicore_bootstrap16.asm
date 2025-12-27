BITS 16

global core_bootstrap
extern core_has_booted 
extern master_tells_core_to_start
extern new_gdt

section .bss.multicore_bootstrap16
section .data.multicore_bootstrap16
section .rodata.multicore_bootstrap16


section .text.multicore_bootstrap16
core_bootstrap: 


