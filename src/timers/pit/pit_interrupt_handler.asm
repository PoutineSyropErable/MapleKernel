BITS 32
extern kprintf2


section .rodata
NEWLINE equ 10
; interrupt_printf_fmt db "In Interrupt handler: %d", NEWLINE, NEWLINE, 0


interrupt_printf_fmt db "In Pit interrupt handler", NEWLINE, 0
PIT_IRQ equ 0


; =================================================== PIT HANDLER =======================================
extern pit_interrupt_handler
global pit_interrupt_handler_asm
pit_interrupt_handler_asm:
	push ebp
	mov ebp, esp

	pusha               ; save registers
    push ds
    push es
    push fs
    push gs

	; push interrupt_printf_fmt 
	; call kprintf2 
	; add esp, 4

	call pit_interrupt_handler


	pop gs
    pop fs
    pop es
    pop ds
    popa

	mov esp, ebp
	pop ebp

	iret

