BITS 16            ; We're in 16-bit real mode
ORG 0x7C00         ; BIOS loads the boot sector here

start:
    mov si, message

print_loop:
    lodsb                   ; Load byte at [SI] into AL and increment SI
    or al, al               ; Check if AL == 0 (null terminator)
    jz hang
    mov ah, 0x0E            ; BIOS teletype output function
    int 0x10                ; BIOS interrupt: print character in AL
    jmp print_loop

hang:
    cli
    hlt
    jmp hang                ; Hang forever

message db "Hello, world!", 0

times 510 - ($ - $$) db 0   ; Pad the rest of the 512 bytes with 0
dw 0xAA55                   ; Boot signature

