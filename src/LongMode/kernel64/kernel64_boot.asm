extern __stack_top

; =====================================================
; Kernel Entry Point
; =====================================================
section .text.entry
global kernel64_start
kernel64_start:
    mov rsp, __stack_top

	
	; Is it proper multiboot? (ECX = (EAX == MB2_MAGIC_EAX))



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

