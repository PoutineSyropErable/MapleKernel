BITS 32
DEFAULT REL

SECTION .multiboot
ALIGN 8

mb2_header_start:
    dd 0xE85250D6            ; magic
    dd 0                    ; arch = i386 (still correct for x86_64)
    dd mb2_header_end - mb2_header_start
    dd -(0xE85250D6 + 0 + (mb2_header_end - mb2_header_start))

    ; Framebuffer request tag
    ; dw 5                    ; MB2_TAG_FRAMEBUFFER
    ; dw 0
    ; dd 24
    ; dd 0                    ; width = any
    ; dd 0                    ; height = any
    ; dd 32                   ; bpp
    ; dd 0

    ; End tag
    dw 0
    dw 0
    dd 8
mb2_header_end:


SECTION .bss
ALIGN 16
stack_bottom:
    resb 16384
stack_top:


SECTION .rodata
msg: db "Hello COM1!", 0
start_msg: db 10, 10, "===================== Start of ==================",10,10, 0


SECTION .text
GLOBAL _start
extern kernel_main


; COM1 base port
%define COM1 0x3F8

_start:
    ; rdi = multiboot2 info pointer
    ; rax = multiboot magic (0x36d76289)

    mov ebp, 0
    mov esp, stack_top

	call kernel_main


.hang:
    hlt
    jmp .hang


; =================================



; ; Wait until transmit buffer is empty
; wait_tx:
;     mov dx, COM1 + 5      ; Line Status Register
; 	mov bl, al
; .tx_empty:
;     in al, dx
;     test al, 0x20         ; Transmit Holding Register Empty
;     jz .tx_empty
; 	mov al, bl
;     ret
;
; ; Write a byte in AL to COM1
; write_com1:
;     call wait_tx
;     mov dx, COM1
;     out dx, al
;     ret
;
;
; global write_string
; write_string:
;     mov esi, rdi          ; rdi points to string
; .next_char:
; 	mov al, [esi]
;     test al, al
;     jz .done
;     call write_com1
; 	inc esi
;     jmp .next_char
; .done:
;     ret



global init_com1
init_com1:
    mov dx, COM1 + 1      ; Line Control Register
    mov al, 0x00          ; Disable interrupts
    out dx, al

    mov dx, COM1 + 3      ; Line Control Register
    mov al, 0x80          ; Enable DLAB
    out dx, al

    mov dx, COM1 + 0      ; Divisor LSB (115200 baud)
    mov al, 0x01
    out dx, al

    mov dx, COM1 + 1      ; Divisor MSB
    mov al, 0x00
    out dx, al

    mov dx, COM1 + 3      ; Line Control Register
    mov al, 0x03          ; 8 bits, no parity, 1 stop bit
    out dx, al

    mov dx, COM1 + 2      ; FIFO Control Register
    mov al, 0xC7          ; Enable FIFO, clear them, 14-byte threshold
    out dx, al

    mov dx, COM1 + 4      ; Modem Control Register
    mov al, 0x0B          ; IRQs enabled, RTS/DSR set
    out dx, al

    ret
; =================================
; draw_framebuffer:
;     mov ebx, rdi            ; mb2 info pointer
;     add ebx, 8              ; skip total_size + reserved
;
; .next_tag:
;     mov eax, [ebx]
;     cmp eax, 0
;     je .done
;
;     cmp eax, 8              ; framebuffer info tag
;     je .fb_found
;
;     mov ecx, [ebx + 4]      ; size
;     add ecx, 7
;     and ecx, -8
;     add ebx, ecx
;     jmp .next_tag
;
; .fb_found:
;     mov esi, [ebx + 8]      ; framebuffer address
;     mov ecx, [ebx + 16]     ; pitch
;     mov edx, [ebx + 20]     ; width
;     mov r8d, [ebx + 24]     ; height
;
;     ; Fill screen
;     xor r9d, r9d            ; y = 0
;
; .y_loop:
;     xor r10d, r10d          ; x = 0
;
; .x_loop:
;     ; pixel = (x ^ y) * 0x010101
;     ; mov eax, r10d
;     ; xor eax, r9d
;     ; imul eax, 0x010101
; 	mov eax, 0xffbbcc
;
; 	mov esi, 0xfd000000
;     mov [esi + r10*4], eax
;
;     inc r10d
;     cmp r10d, edx
;     jl .x_loop
;
;     add esi, ecx
;     inc r9d
;     cmp r9d, r8d
;     jl .y_loop
;
; .done:
;     ret


