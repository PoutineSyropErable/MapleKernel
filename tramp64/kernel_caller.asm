BITS 64
section .text.trampoline

global kernel_caller
kernel_caller:
    ; compute address of kernel64_main at runtime
    mov rax, [rel kernel_main_64_ptr]  ; load from pointer
    test rax, rax
    jz .no_kernel64
    jmp rax

.no_kernel64:
    hlt
    jmp .no_kernel64

section .data
kernel_main_64_ptr: dq 0      ; first link: zero

