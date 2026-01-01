BITS 32
extern kprintf2
extern apic_send_eoi
extern apic_wait_interrupt_handled


section .rodata
NEWLINE equ 10
; interrupt_printf_fmt db "In Interrupt handler: %d", NEWLINE, NEWLINE, 0


interrupt_printf_fmt db "In apic interrupt handler", NEWLINE, 0
apic_IRQ equ 0



%define LAPIC_BASE        0xFEE00000  ; typical default LAPIC base
%define EOI_OFFSET        0xB0  ; typical default LAPIC base
%assign EOI_MMIO_ADDR LAPIC_BASE + EOI_OFFSET

; =================================================== apic HANDLER =======================================
extern apic_interrupt_handler
global apic_interrupt_handler_asm
apic_wait_interrupt_handler_asm:

	push eax

	mov eax, fs:[0] ; get the core ID
	mov [apic_wait_interrupt_handled + eax], 1
	mov dword [ EOI_MMIO_ADDR ], 0
	; DWORD WRITE. Without it, it writes a byte. And the eoi isn't properly recieved



	pop eax

	iret


