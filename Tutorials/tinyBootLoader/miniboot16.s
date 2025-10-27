bits 16

; When you turn your computer on, the processor immediately looks at physical address 0xFFFF_FFF0 (4 gigs - 16 bytes) for the BIOS code,
; which is generally on some read-only piece of ROM somewhere in your computer.

; The BIOS then POSTs (Power On Self Test: Check the hardware, initialize device, report errors), and searches for acceptable boot media.
; The BIOS accepts some medium as an acceptable boot device if its boot sector, 
; the first 512 bytes of the disk are readable and end in the exact bytes 0x55AA, which constitutes the boot signature for the medium.
; If the BIOS deems some drive bootable, then it loads the first 512 bytes of the drive into memory address 0x007C00,
; and transfers program control to this address with a jump instruction to the processor.


; A boot sector is the 512 first bytes of a drive. 
; boot code = ~446 bytes 
; partition table = 64 bytes 
; boot signature = 2 bytes. (MUST BE 0x55AA)
; The bios thankfully offers some minimal driver to read from a disk. 
; This allows you to read from /boot partition. Which allows you to copy stuff to ram



; 7c0 = 7 12 0 = 0 + 12*16 + 7*256  = 1984 
; 7c = 124
; 7c00 = 31744
; The choice of that number was just so that there's enough place for the idt bellow, and enough place for the stack above. 
; But that was chosen in 1981, so if we round up for 16 | x, we get 1984
; 1981 = 0x7bd
mov ax, 0x7c0
mov ds, ax 
; set the data segment to 0x7c0, so The segment base is 0x7c00

; cs = 0x0000, already set to that by the bios. 

; The bootloader will copy the first 512 bytes. The stack is 512 bytes after it. 0x07c0 + 512 bytes = 0x07e0
; But that is used for the partition and multiboot too.
mov ax, 0x7E0
mov ss, ax
; ss is the base of the sack. And 0x800 (sp set bellow) is the top. So, the stack will never touch the multiboot header copied to memory


; I put it lower then the tutorial, to follow the .md scheme i used right before
mov sp, 0x800

call clearscreen

push 0x0000 
call movecursor 
add sp, 2

push msg 
call print 
add sp, 2 

cli 
hlt


; void clearscreen() {... }
; calls bios_clear_screen through interrupt table. 

; void bios_clear_screen(uint8_t numberLinesToScroll, uint8_t colorings)
clearscreen:
    push bp
    mov bp, sp
    pusha				; Push all general registers to the stack (ax, cx, dx, bx, sp, bp, si, di)

	; The screen size is  25/80,

	; https://www.ctyme.com/intr/rb-0097.html
    mov ah, 0x07        ; tells BIOS to scroll down window (ah = function number)
    mov al, 0x00        ; clear entire window			   (al = arg1)
    mov bh, 0x07        ; white on black
	; https://en.wikipedia.org/wiki/BIOS_color_attributes
	; Low 4 bit = character
	; High 4 bit = background
	; 0 = black, 7 = light gray. 
	; {bg = black, fg = light_gray}
    mov cx, 0x0000      ; specifies top left of screen as (0,0)
	; cx = ch:cl = row:column of upper right corner
    mov dh, 0x18        ; 18h = 24 rows of chars
    mov dl, 0x4f        ; 4fh = 79 cols of chars
	; dx = dh:dl = row:column of lower right corner
    int 0x10        	; calls video interrupt

    popa                ; restore all general registers
    mov sp, bp
    pop bp
    ret


movecursor:
    push bp
    mov bp, sp
    pusha

    mov dx, [bp+4]      ; get the argument from the stack. |bp| = 2, |arg| = 2
    mov ah, 0x02        ; set cursor position
    mov bh, 0x00        ; page 0 - doesn't matter, we're not using double-buffering
    int 0x10

    popa
    mov sp, bp
    pop bp
    ret



print:
    push bp
    mov bp, sp
    pusha

    mov si, [bp+4]      ; grab the pointer to the data
    mov bh, 0x00        ; page number, 0 again
    mov bl, 0x00        ; foreground color, irrelevant - in text mode
    mov ah, 0x0E        ; print character to TTY
.char:
    mov al, [si]        ; get the current char from our pointer position
	add si, 1           ; keep incrementing si until we see a null char
    or al, 0			; check if al == NULL 
    jz .return          ; end if the string is done
    int 0x10            ; print the character if we're not done
    jmp .char       ; keep looping
.return:
     popa
     mov sp, bp
     pop bp
     ret

msg:    db "Oh boy do I sure love assembly!", 0

; 206 bytes. Not so much. I can add some function to read from the drive. 
times 446-($-$$) db 0
; the data of the partition table

; MBR partition table (16 bytes each)
; entry format: boot(1B), start CHS(3B), type(1B), end CHS(3B), start LBA(4B), size in sectors(4B)

; Partition 1: bootable FAT32
db 0x80, 0x00,0x02,0x00   	; boot + start CHS (dummy)
db 0x0B                     ; type FAT32
db 0x00,0x00,0x00           ; end CHS (dummy)
dd 0x00000001               ; start LBA
dd 100                      ; size in sectors

; Partition 2: Linux
db 0x00, 0x00,0x00,0x00
db 0x83
db 0x00,0x00,0x00
dd 101
dd 200

; Partition 3: Linux
db 0x00, 0x00,0x00,0x00
db 0x83
db 0x00,0x00,0x00
dd 301
dd 300

; Partition 4: unused
dq 0x0000000000000000       ; 8 bytes
dq 0x0000000000000000       ; 8 bytes

dw 0xAA55
