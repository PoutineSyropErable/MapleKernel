; -------------------------- boot64.s --------------------------
section .text64
BITS 64
long_mode_entry:
    mov rdi, 0xB8000        ; VGA text buffer
    lea rsi, [rel msg]

.loop:
    lodsb
    or al, al
    jz .done
    mov [rdi], ax
    add rdi, 2
    jmp .loop

.done:
    hlt

section .data64
msg db 'Hello World',0

