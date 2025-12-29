BITS 32
DEFAULT REL

SECTION .multiboot
ALIGN 8

mb2_header_start:
    dd 0xE85250D6            ; magic
    dd 0                    ; arch = i386 (still correct for x86_64)
    dd mb2_header_end - mb2_header_start
    dd -(0xE85250D6 + 0 + (mb2_header_end - mb2_header_start))

    ; Framebuffer request tag
    ; dw 5                    ; MB2_TAG_FRAMEBUFFER
    ; dw 0
    ; dd 24
    ; dd 0                    ; width = any
    ; dd 0                    ; height = any
    ; dd 32                   ; bpp
    ; dd 0

    ; End tag
    dw 0
    dw 0
    dd 8
mb2_header_end:


SECTION .bss
ALIGN 16
stack_bottom:
    resb 16384
stack_top:


SECTION .rodata
msg: db "Hello COM1!", 0
start_msg: db 10, 10, "===================== Start of ==================",10,10, 0


SECTION .text
GLOBAL _start
extern kernel_main_32



_start:
    ; rdi = multiboot2 info pointer
    ; rax = multiboot magic (0x36d76289)

    mov ebp, 0
    mov esp, stack_top

	call kernel_main_32


.hang:
    hlt
    jmp .hang
