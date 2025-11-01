;add16_wrapper16.s
BITS 16

global add1616_start
extern add16
extern args16_start
extern resume32

section .text.add1616

add1616_start:

	mov eax, 0xdeadfac2
	


    ; Load arguments from buffer
    ; mov ax, [args16_start+4]       ; first argument
    ; mov bx, [args16_start+6]     ; second argument


	; hlt
	; jmp add1616_start

	; push ax 
	; push bx
    ; call add16                    ; call actual 16-bit add
	; mov ax, 15
    ; mov [args16_start+8], ax      ; store result
	; pop bx 
	; pop ax



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



; halt_loop: 
; 	hlt 
; 	jmp halt_loop
;
;
; reset:
; 	cli                     ; disable interrupts
; 	xor eax, eax
; 	lidt [eax]              ; load IDT base = 0, limit = 0 (invalid)
; 	int 3                   ; trigger interrupt -> #GP -> #DF -> triple fault
