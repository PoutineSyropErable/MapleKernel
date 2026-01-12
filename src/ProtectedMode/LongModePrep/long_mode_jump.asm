;long_mode_jump.asm
BITS 32


extern gdtr_64 
extern idtr_64
extern cr3_of_setup

extern k64


section .bss
    align 16
mem_stack_bottom:
    resb 16384             ; reserve 16 KiB
meh_stack_top:

section .data

section .rodata


section .text
extern compatibility_entry

PAE_MASK equ 1 << 5
PG_MASK equ 1 << 31
WRITE_PROTECT_MASK equ 1 << 16

EFER_MSR equ 0xC0000080
EFER_LM_ENABLE equ 1 << 8


EFER_NXE_ENABLE equ (1 << 11)


global to_compatibility_mode
to_compatibility_mode: 

	; efer.lme =1 ,
	; cr4.pae = 1,
	; cr0.pg = 1

	; Setup paging
	; set cr3 -> pml4
	; cr4.pae = 1, 
	; efer.lme = 1
	; cr0.pg = 1
	; gdt 
	; idt
	; far jump to 64

	cli
	push dword k64


	mov eax, dword [cr3_of_setup]
	mov cr3, eax

	mov eax, cr4
	or eax, PAE_MASK   ; set bit 5 = PAE
	mov cr4, eax

    mov ecx, EFER_MSR
    rdmsr
    or eax, EFER_LM_ENABLE     ; Enable long mode
	or eax, EFER_NXE_ENABLE    ; Set NXE bit
    wrmsr



	mov eax, cr0
	or eax, PG_MASK 
	or eax, WRITE_PROTECT_MASK
	mov cr0, eax

	lidt [idtr_64]
	lgdt [gdtr_64]

	jmp far 0x08:bridge64
	; I should jump to some 64 bit code immediatly. 
	; Not the compatibility entry
	; jmp far 0x18:compatibility_entry
	; where is ds, es fs, and gs set to 0x20?
	; it's set when you enter compatibility_entry

	
	
section .text 
bridge64:
incbin "bridge64.bin"


; ======================= END =============
section .text
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

