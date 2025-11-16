BITS 32
extern exception_handler
extern kprintf_argc

section .rodata
argc dd 2
NEWLINE equ 10
interrupt_printf_fmt db "In Interrupt handler: %d", NEWLINE, NEWLINE, 0
interrupt_69_fmt db NEWLINE, "Hello from Inside the Interrupt handler %d, nice!", NEWLINE, NEWLINE, 0

section .text


hlt_loop:
	mov eax, 0x1234abcd
	cli
	hlt
	jmp hlt_loop



; if writing for 64-bit, use iretq instead
%macro isr_stub 1
isr_stub_%+%1:
	push ebp
	mov ebp, esp

	push %1 
	push interrupt_printf_fmt  
	push [argc] 
	call kprintf_argc
	; add esp, 12 ; needed if i don't do the prologue and epilogue

	mov esp, ebp
	pop ebp
	; mov eax, 1
	iret
%endmacro

isr_stub 0
isr_stub 1
isr_stub 2
isr_stub 3
isr_stub 4
isr_stub 5
isr_stub 6
isr_stub 7
isr_stub 8
isr_stub 9
isr_stub 10
isr_stub 11
isr_stub 12
isr_stub 13
isr_stub 14
isr_stub 15
isr_stub 16
isr_stub 17
isr_stub 18
isr_stub 19
isr_stub 20
isr_stub 21
isr_stub 22
isr_stub 23
isr_stub 24
isr_stub 25
isr_stub 26
isr_stub 27
isr_stub 28
isr_stub 29
isr_stub 30
isr_stub 31



global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep


global interrupt_34_handler 
interrupt_34_handler: 
	push ebp
	mov ebp, esp

	push 34 
	push interrupt_printf_fmt  
	push [argc] 
	call kprintf_argc
	add esp, 12

	mov esp, ebp
	pop ebp
	; mov eax, 1
	iret


global interrupt_69_handler 
interrupt_69_handler: 
	push ebp
	mov ebp, esp

	push 69
	push interrupt_69_fmt
	push [argc] 
	call kprintf_argc
	add esp, 12

	mov esp, ebp
	pop ebp
	; mov eax, 1
	iret



interrupt_8_fmt db "In Interrupt handler: %d, DOUBLE_FAULT", NEWLINE, 0
err_inf_fmt db "Error Code = %u, EIP = %u, CS = %u, Eflags = %b" , NEWLINE, 0

global interrupt_8_handler 
interrupt_8_handler: 
	push ebp
	mov ebp, esp

	pusha 
	push ds 
	push es 
	push fs 
	push gs 


	;; argc = number of % + 1; 
	mov eax, [ebp +4]   ; error code
	mov ebx, [ebp + 8]  ; EIP 
	mov ecx, 0
	mov cx, word [ebp + 12] ; CS
	mov edx, [ebp + 16] ; Eflags


	push edx
	push ecx
	push ebx
	push eax
	push err_inf_fmt 
	push 5 ; argc 
	call kprintf_argc
	add esp, 24

	push 8 ; %d
	push interrupt_8_fmt ; fmt
	push 2 ; argc
	call kprintf_argc
	add esp, 12


	pop gs 
	pop fs 
	pop es
	pop ds 
	popa


	mov esp, ebp
	pop ebp
	iret
