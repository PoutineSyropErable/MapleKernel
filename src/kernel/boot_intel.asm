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
	mov edx, 0xdeadfac0
    mov esp, stack_top

	
	; Is it proper multiboot? (ECX = (EAX == MB2_MAGIC_EAX))
%define MB2_MAGIC_EAX 0x2BADB002
	cmp eax, MB2_MAGIC_EAX
	sete cl 
	movzx ecx, cl



    extern kernel_main
    ; EBX = pointer to Multiboot2 info structure
	; EAX = MAGIC value
	push ecx ; (proper multiboot)
	push eax ; magic number
	push ebx ; push mb2_info_addr
    call kernel_main
	add esp, 12

    ; Infinite halt loop if kernel_main returns
halt_loop:
    cli                     ; clear interrupt flag
    hlt                     ; halt CPU
    jmp halt_loop           ; jump back to halt

; Set symbol size (optional, useful for debugging)
; size _start, . - _start
