BITS 32


section .rodata
NEWLINE equ 10
interrupt_printf_fmt db "In Interrupt handler: %d", NEWLINE, NEWLINE, 0
interrupt_69_fmt db NEWLINE, "Hello from Inside the Interrupt handler %d, nice!", NEWLINE, NEWLINE, 0


section .text

; =================================================== KEYBOARD =======================================
extern keyboard_handler
global keyboard_interrupt_handler_port1
keyboard_interrupt_handler_port1:
	push ebp
	mov ebp, esp

	pusha               ; save registers
    push ds
    push es
    push fs
    push gs

	mov eax, 0
PS2_DATA_PORT_RW equ 0x60
	in al, PS2_DATA_PORT_RW
	push eax 
	call keyboard_handler
	add esp, 4


	pop gs
    pop fs
    pop es
    pop ds
    popa

	mov esp, ebp
	pop ebp

	iret



extern keyboard_handler
global keyboard_interrupt_handler_port2
keyboard_interrupt_handler_port2:
	push ebp
	mov ebp, esp

	pusha               ; save registers
    push ds
    push es
    push fs
    push gs

	mov eax, 0
PS2_DATA_PORT_RW equ 0x60
	in al, PS2_DATA_PORT_RW
	push eax 
	call keyboard_handler
	add esp, 4


	pop gs
    pop fs
    pop es
    pop ds
    popa

	mov esp, ebp
	pop ebp

	iret





; =================================================== MOUSE =======================================
extern mouse_handler
global mouse_interrupt_handler_port1
mouse_interrupt_handler_port1:
	push ebp
	mov ebp, esp

	pusha               ; save registers
    push ds
    push es
    push fs
    push gs

	mov eax, 0
PS2_DATA_PORT_RW equ 0x60
	in al, PS2_DATA_PORT_RW
	push eax 
	call mouse_handler
	add esp, 4


	pop gs
    pop fs
    pop es
    pop ds
    popa

	mov esp, ebp
	pop ebp

	iret



extern mouse_handler
global mouse_interrupt_handler_port2
mouse_interrupt_handler_port2:
	push ebp
	mov ebp, esp

	pusha               ; save registers
    push ds
    push es
    push fs
    push gs

	mov eax, 0
PS2_DATA_PORT_RW equ 0x60
	in al, PS2_DATA_PORT_RW
	push eax 
	call mouse_handler
	add esp, 4


	pop gs
    pop fs
    pop es
    pop ds
    popa

	mov esp, ebp
	pop ebp

	iret
