.intel_syntax noprefix
; =====================================================
; Multiboot Header Constants
; =====================================================
ALIGN    equ 1<<0          ; align loaded modules on page boundaries
MEMINFO  equ 1<<1          ; provide memory map
FLAGS    equ ALIGN | MEMINFO ; multiboot 'flags'
MAGIC    equ 0x1BADB002    ; magic number
CHECKSUM equ -(MAGIC + FLAGS) ; checksum

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
    mov esp, stack_top

    ; Call the high-level kernel entry point
    call kernel_main

; Infinite halt loop if kernel_main returns
halt_loop:
    cli                     ; clear interrupt flag
    hlt                     ; halt CPU
    jmp halt_loop           ; jump back to halt

; Set symbol size (optional, useful for debugging)
; size _start, . - _start

