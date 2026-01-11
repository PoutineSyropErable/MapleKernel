extern __stack_top
extern __module_size

extern __rodata_start



; =====================================================
; Kernel Entry Point
; =====================================================
section .rodata
msg db "Hello", 10, 0

section .text.entry
global kernel64_start
kernel64_start:
    ; Set stack pointer (stack grows downward)
	; db 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x95, 0x97  ; 8 NOPs in a row
    mov rsp, __stack_top

	mov [msg], 0xab
	mov ds:[msg], 0xab
	mov dil, ds:[msg]

	mov rax, 0x123
	mov [__rodata_start], rax
	; mov dil, [msg]
	movzx rdi, dil
	call fill_framebuffer



extern kernel64_main
    ; call kernel64_main

    ; Infinite halt loop if kernel_main returns
halt_loop64:
    cli                     ; clear interrupt flag
    hlt                     ; halt CPU
    jmp halt_loop64           ; jump back to halt

; Set symbol size (optional, useful for debugging)
; size _start, . - _start


FB_BASE equ 0xffff888000000000
FB_PIXELS equ 1920 * 1080

fill_framebuffer:
    mov rbx, FB_BASE
    mov rcx, FB_PIXELS      ; Pixel count, not byte address!
    
.fill_loop:
    test rcx, rcx
    jz .end
    
    mov [rbx], edi          ; Write 32-bit color
    add rbx, 4
    dec rcx
    jmp .fill_loop
    
.end:
    ret
