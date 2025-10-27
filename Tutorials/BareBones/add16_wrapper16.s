BITS 16
section .text.add1616
global add1616_start
extern add16
extern args16_start
extern resume32

add1616_start:
	hlt
	jmp add1616_start
    ; Load arguments from buffer
    mov ax, [args16_start]       ; first argument
    mov bx, [args16_start+2]     ; second argument

    call add16                    ; call actual 16-bit add

    mov [args16_start+4], ax      ; store result

    ; --- Done, return to 32-bit wrapper ---
    jmp resume32                  ; far jump is unnecessary, 32-bit wrapper handles CR0

