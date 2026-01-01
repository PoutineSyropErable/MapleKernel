BITS 32
extern kprintf2

extern quick_pit
extern pit_interrupt_handled


section .rodata
NEWLINE equ 10
; interrupt_printf_fmt db "In Interrupt handler: %d", NEWLINE, NEWLINE, 0


interrupt_printf_fmt db "In Pit interrupt handler", NEWLINE, 0
PIT_IRQ equ 0


%define LAPIC_BASE        0xFEE00000  ; typical default LAPIC base
%define EOI_OFFSET        0xB0  ; typical default LAPIC base
%assign EOI_MMIO_ADDR LAPIC_BASE + EOI_OFFSET

; =================================================== PIT HANDLER =======================================
extern pit_interrupt_handler
global pit_interrupt_handler_asm
pit_interrupt_handler_asm:
	push eax
	mov eax, [quick_pit]
	test eax, eax ; if quick pit, then not zero
	jz .long_pit

	mov eax, 1
	mov [pit_interrupt_handled], eax
	mov [ EOI_MMIO_ADDR ], 0

	pop eax
	iret



.long_pit:
	pop eax

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


	iret

