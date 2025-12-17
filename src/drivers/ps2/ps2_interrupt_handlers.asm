BITS 32


section .rodata
NEWLINE equ 10
interrupt_printf_fmt db "In Interrupt handler: %d", NEWLINE, NEWLINE, 0
interrupt_69_fmt db NEWLINE, "Hello from Inside the Interrupt handler %d, nice!", NEWLINE, NEWLINE, 0


PORT_ONE equ 1
PORT_TWO equ 2
PS2_DATA_PORT_RW equ 0x60


section .bss 
	keyboard1_esp: dw 
	keyboard2_esp: dw 
	mouse1_esp: dw 
	mouse2_esp: dw 

section .text
extern kprintf_argc

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
	in al, PS2_DATA_PORT_RW


	push PORT_ONE
	push eax 
	call keyboard_handler
	add esp, 8


	pop gs
    pop fs
    pop es
    pop ds
    popa

	mov esp, ebp
	pop ebp

	iret



global keyboard_interrupt_handler_port2
keyboard_interrupt_handler_port2:
	push ebp
	mov ebp, esp

	; ALIGN STACK TO 16 BYTES FOR SSE SAFETY
    and esp, -16
    sub esp, 32          ; Allocate space (multiple of 16)


	pusha               ; save registers
    push ds
    push es
    push fs
    push gs

	mov eax, 0
	in al, PS2_DATA_PORT_RW

	push PORT_TWO
	push eax 
	call keyboard_handler
	add esp, 8


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
	in al, PS2_DATA_PORT_RW

	push PORT_ONE
	push eax 
	call mouse_handler
	add esp, 8


	pop gs
    pop fs
    pop es
    pop ds
    popa

	mov esp, ebp
	pop ebp

	iret



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
	in al, PS2_DATA_PORT_RW

	push PORT_TWO
	push eax 
	call mouse_handler
	add esp, 8


	pop gs
    pop fs
    pop es
    pop ds
    popa

	mov esp, ebp
	pop ebp

	iret
