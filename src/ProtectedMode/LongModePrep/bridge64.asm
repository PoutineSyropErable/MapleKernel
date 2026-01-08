;bridge64.asm
[BITS 64]

VIRTUAL_ADDRESS_OFFSET equ 0
SIZE_OFFSET equ 8
PHYSICAL_ADDRESS_OFFSET equ 16

long_mode_entry:
	mov ax, 0x10
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax
	mov ss, ax 

	mov rbx, 0xdeadbeefdeadface
	mov rax, 0xffffffff80000000
	mov ecx, dword [rsp] ; get the address of k64 from the stack
	movzx rcx, ecx

	mov rax, [rcx + VIRTUAL_ADDRESS_OFFSET] ; get the virtual address of the kernel
	mov ebx, [rcx + PHYSICAL_ADDRESS_OFFSET] ; get the virtual address of the kernel
	movzx rbx, ebx
	; rax = 0xffffffff80000000 (gotten through runtime)
	mov rcx, qword [rax]
	mov rdx, qword [rbx]
	; mov rdx, [0xdeadbeefdeadface]
	jmp rax ; This jump to the kernel


	mov rdx, [0xdeadbeefdeadface]



	.hl
	hlt
	jmp .hl 


	

