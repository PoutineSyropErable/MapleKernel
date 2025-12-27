BITS 32

extern core_has_booted 
extern master_tells_core_to_start
extern new_gdt
extern core_mains
; extern kernel_main

extern __new_gdt_start
extern __new_gdt_end


%define MAX_CPU_COUNT 8
%define STACK_SIZE 16384 ; 16kb
%assign MULTISTACK_SIZE STACK_SIZE * MAX_CPU_COUNT

section .bss.multicore_bootstrap32
    align 16
multistack_bottom:
    resb MULTISTACK_SIZE             ; reserve 16 KiB
multistack_top:

section .data.multicore_bootstrap32

section .rodata.multicore_bootstrap32


section .text.multicore_bootstrap32
global core_bootstrap32

%define FRAMEBUFFER 0xFD000000

; LAPIC registers (offsets)
%define LAPIC_BASE        0xFEE00000  ; typical default LAPIC base
%define LAPIC_ICR_LOW     0x300
%define LAPIC_ICR_HIGH    0x310

%define LAPIC_ID_OFFSET  0x20
%define LAPIC_ID_ADDR  0xFEE00020

%define DELIVERY_MODE_INIT 0x5
%define DELIVERY_MODE_SIPI 0x6
%define DEST_PHYSICAL      0x0
core_bootstrap32: 
	mov eax, dword [ LAPIC_BASE +  LAPIC_ID_OFFSET ]
	shr eax, 24 ; apic id

	mov ebx, eax 
	shl ebx, 1  ; apic id *2
	add ebx, 4  ; gdt idx of fs
	mov ecx, ebx ; copy for gs
	shl ebx, 3  ; fs
	mov fs, bx ; set fs
	add ecx, 1 ; gdt idx of gs
	shl ecx, 3 ; gs value
	mov gs, cx 

	mov fs:[0], eax; quick save apic id

	mov edx, [core_mains + 4*eax] ; core_main
	mov [core_has_booted + eax], 1;

	; mov eax, multistack_top	
	; jmp fill_color_eax

	mov esp, multistack_top
	imul eax, STACK_SIZE
	sub esp, eax


	push eax
	call edx
	pop eax

	; mov eax, 0x00FFBBCC


	


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





; -------------------------------------------------------
; Send SIPI to core ID 1 with vector 7
; -------------------------------------------------------

; LAPIC registers (offsets)
%define LAPIC_BASE        0xFEE00000  ; typical default LAPIC base
%define LAPIC_ICR_LOW     0x300
%define LAPIC_ICR_HIGH    0x310

%define DELIVERY_MODE_INIT 0x5
%define DELIVERY_MODE_SIPI 0x6
%define DEST_PHYSICAL      0x0





