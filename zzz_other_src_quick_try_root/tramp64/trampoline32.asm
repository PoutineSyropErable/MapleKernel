BITS 32
section .text.trampoline
extern kernel_caller  

global jump_to_64bit
jump_to_64bit:
	; Switch to 64 bit
	; enable paging, efer.lme, other stuff 
	; Not doing that, since this is a linking exercise, not a 
	; make it work at runtime exercise



	jmp kernel_caller
