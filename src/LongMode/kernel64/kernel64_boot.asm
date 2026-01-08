extern __stack_top
extern __module_size

; =====================================================
; Kernel Entry Point
; =====================================================
section .text.entry
global kernel64_start
kernel64_start:
    ; Set stack pointer (stack grows downward)
	; db 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x95, 0x97  ; 8 NOPs in a row
    mov rsp, __stack_top
	mov rax, __module_size

    extern kernel64_main
    ; EBX = pointer to Multiboot2 info structure
	; EAX = MAGIC value
    call kernel64_main

    ; Infinite halt loop if kernel_main returns
halt_loop64:
    cli                     ; clear interrupt flag
    hlt                     ; halt CPU
    jmp halt_loop64           ; jump back to halt

; Set symbol size (optional, useful for debugging)
; size _start, . - _start

