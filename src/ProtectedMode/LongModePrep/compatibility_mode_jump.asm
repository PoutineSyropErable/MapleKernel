BITS 32


extern gdtr_64 
extern idtr_64
extern cr3_of_setup


section .bss
    align 16
mem_stack_bottom:
    resb 16384             ; reserve 16 KiB
meh_stack_top:

section .data

section .rodata


section .text

%define PAE_MASK 1 << 5
global compatibility_entry
compatibility_entry: 
	mov ax, 0x20 
	mov ds, ax 
	mov es, ax 
	mov ss, ax 
	mov fs, ax 
	mov gs, ax

	


	
	


%define FRAMEBUFFER 0xFD000000
fill_color_eax:
	mov ebx, 0
	mov edi, FRAMEBUFFER
	mov ecx, 1024*768
	.fill:
		mov [edi], eax
		add edi, 4
		add ebx, 1
		cmp ebx, ecx 
		jnz .fill

	hl: 
		jmp hl











