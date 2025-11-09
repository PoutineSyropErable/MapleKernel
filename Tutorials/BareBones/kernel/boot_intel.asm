; =====================================================
; Multiboot Header Constants
; =====================================================
%define ALIGN    1
%define MEMINFO  2  
%define FLAGS    3
%define MAGIC    0x1BADB002
%define CHECKSUM 0xE4524FFB

; =====================================================
; Multiboot Header Section
; =====================================================
section .multiboot
    align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; =====================================================
; Stack Section (16-byte aligned, 16 KiB)
; =====================================================
section .bss
    align 16
stack_bottom:
    resb 16384              ; reserve 16 KiB
stack_top:

; =====================================================
; Kernel Entry Point
; =====================================================
section .text
global _start
_start:
    ; Set stack pointer (stack grows downward)
	mov eax, 0xdeadfac0
    mov esp, stack_top

    ; Call the high-level kernel entry point
    extern kernel_main
    call kernel_main

    ; Infinite halt loop if kernel_main returns
halt_loop:
    cli                     ; clear interrupt flag
    hlt                     ; halt CPU
    jmp halt_loop           ; jump back to halt

; Set symbol size (optional, useful for debugging)
; size _start, . - _start
