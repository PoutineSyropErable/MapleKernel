;add16_wrapper32.s
BITS 32

global call_add16
global resume32
extern add1616_start

extern stack16_start
extern stack16_end
extern args16_start

section .text.add1632

call_add16:
	mov eax, 0xdeadfac1

    ; Save 32-bit registers and flags
    pushad
    pushfd
    push ds
    push es
    push fs
    push gs

	; rdi = arg1 
	; rsi = arg2
	; Save the stack pointer in the first 1mb (first 64kb in fact)
	; So its accessible in 16 bit, and can be restored on the way back to 32 bit
	mov [args16_start], esp      ; 
	mov [args16_start +4], edi      ; 
	mov [args16_start +8], esi      ; 


    cli


	; mov ax, 0
	; mov ds, ax
	; mov es, ax
	; mov fs, ax
	; mov gs, ax
	;
 ;    ; Set up 16-bit stack
	; mov eax, 0
 ;    mov eax, stack16_start
 ;    shr eax, 4             ; segment = address >> 4
 ;    mov ss, ax
	; mov esp, 0x4000

    ; --- Switch to 16-bit mode ---

    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    ; Far jump to 16-bit wrapper
	jmp far 00:add1616_start



halt_loop: 
	hlt 
	jmp halt_loop
;
; reset:
; 	cli                     ; disable interrupts
; 	xor eax, eax
; 	lidt [eax]              ; load IDT base = 0, limit = 0 (invalid)
; 	int 3                   ; trigger interrupt -> #GP -> #DF -> triple fault





section .text.resume32
; resume32 will be called by the 16-bit code when done
resume32:
    ; Restore segment registers


    mov esp, [args16_start]

    pop gs
    pop fs
    pop es
    pop ds

    ; Restore general-purpose registers and flags
    popfd
    popad


    ; Retrieve result
    movzx eax, word [args16_start + 8]
	; mov eax, 15

    ret


halt_loop2: 
	hlt 
	jmp halt_loop2

