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

section .text

extern keyboard_handler
extern mouse_handler

%macro INTERRUPT_HANDLER 2  ; %1=port, %2=handler_name
global %2_interrupt_handler_port%1
%2_interrupt_handler_port%1:
    push ebp
    mov ebp, esp

	sub esp,4 ; Location to save esp
    
    ; Save all registers
    pusha
    push ds
    push es
    push fs
    push gs

	; Save original ESP at [EBP-4] (safe from nested interrupts)
    mov [ebp-4], esp
    
    ; Align stack for C call (SSE safety)
    and esp, -16
    sub esp, 32
    
    
    ; Get data and call handler
    mov eax, 0
    in al, PS2_DATA_PORT_RW
    push dword %1         ; Port number
    push eax			  ; scancode/data
    call %2_handler       ; C function
    add esp, 8
    
    ; Restore original ESP
	mov esp, [ebp-4]

    
    ; Restore registers
    pop gs
    pop fs
    pop es
    pop ds
    popa

	; Undo saved esp location
	add esp, 4

    mov esp, ebp
    pop ebp
    iret
%endmacro

INTERRUPT_HANDLER 1, keyboard
INTERRUPT_HANDLER 2, keyboard  
INTERRUPT_HANDLER 1, mouse
INTERRUPT_HANDLER 2, mouse
