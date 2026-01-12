extern __stack_top
extern __module_size

extern __rodata_start



; =====================================================
; Kernel Entry Point
; =====================================================
section .rodata
msg db "Hello", 10, 0

section .bss
; must use .text because its the only properly mapped section
entry_virtual: dq 0
entry_physical: dq 0

section .text.entry
global kernel64_start
kernel64_start:
    ; Set stack pointer (stack grows downward)
    mov rsp, __stack_top

	; rax = entry virtual
	; rbx = entry_physical 
	mov [entry_virtual], rax
	mov [entry_physical], rbx

	jmp b
	b:


	mov rcx, msg
	sub rcx, rax 
	add rcx, rbx
	; RCX=000000000010d004
	; phys address of the message

	; jmp crash


	; mov dil, [msg]
	;
	; movzx rdi, dil
	; call fill_framebuffer




extern kernel64_main
    call kernel64_main

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



crash: 
	mov rsi, [0xAAAAAAAAAAAAAAAA]
	ret
