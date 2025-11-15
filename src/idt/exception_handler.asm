BITS 32
extern exception_handler
extern kprintf_argc

section .rodata
argc dd 2
NEWLINE equ 10
interrupt_printf_fmt db "In Interrupt handler: %d", NEWLINE, NEWLINE, 0

section .text


hlt_loop:
	cli
	hlt
	jmp hlt_loop



; if writing for 64-bit, use iretq instead
%macro isr_stub 1
isr_stub_%+%1:
	push %1 
	push interrupt_printf_fmt  
	push [argc] 
	call kprintf_argc
	add esp, 12
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


