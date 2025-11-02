;add16_wrapper16.s
BITS 16

global add1616_start
extern add16
extern args16_start
extern resume32


add1616_start:

	mov bx, 0xfac2
	
    ; --- Done, return to 32-bit wrapper ---
    mov eax, cr0
    or eax, 1
    mov cr0, eax



	mov esp, [ds:args16_start]
	mov ax, 0x18 
	mov ss, ax    ; ss = 0x18 is the 32 bit ss value
	mov ds, ax

	; mov eax, 0x7ef15e32
    jmp far 0x10:resume32                  ; far jump is unnecessary, 32-bit wrapper handles CR0
	; From doing print on grub, 0x10 is the cs code segment in 32 bit. 
	; It's ring 0, gdt, index 2. (Index one is not used. Maybe a grub internal. index 0 is null by hardware need)



halt_loop3: 
	hlt 
	jmp halt_loop3
