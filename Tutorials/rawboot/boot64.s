[BITS 64]
long_mode_entry:
    mov rdi, 0xB8000           ; VGA text buffer
    lea rsi, [rel msg]         ; RIP-relative load of msg address

.loop:
    lodsb                       ; load byte from [rsi] into al, increment rsi
    or al, al                   ; check for null terminator
    jz .done
    mov [rdi], ax               ; write character with attribute
    add rdi, 2                  ; advance VGA cursor (2 bytes per char)
    jmp .loop

.done:
    hlt

section .data
msg db 'Hello World', 0

