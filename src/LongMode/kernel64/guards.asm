; guard.asm - 4KB guard page filled with zeros
; Can be linked multiple times for different guard sections

section .guard
align 4096

; Export a global symbol so we can reference it
global __guard_page_4k
__guard_page_4k:

; Fill with 4096 zeros
times 4096 db 0

; Mark the end
global __guard_page_4k_end
__guard_page_4k_end:
