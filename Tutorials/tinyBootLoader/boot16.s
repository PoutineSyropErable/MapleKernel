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

; The bootloader will copy the first 512 bytes
mov ax, 0x7E0
mov ss, ax


