; org = reset vector physical address
org 0xFFFFFFF0
bits 16

cli
; short jump to start of real mode code
jmp start16

times 16-($-$$) db 0  ; pad to 16 bytes (reset vector)



