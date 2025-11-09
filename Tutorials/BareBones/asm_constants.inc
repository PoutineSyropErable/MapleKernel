; asm_constants.inc
; This is your "header file" for ASM

; Offsets into args16_start
GDT_ROOT_OFFSET   	equ 0
USELESS_PAD_OFFSET 	equ 6
ESP_OFFSET        	equ 8
SS_OFFSET 			equ 12


RET1_OFFSET       equ 14
RET2_OFFSET       equ 16


FUNC_OFFSET       equ 18
FUNC_CS_OFFSET    equ 20


ARGC_OFFSET       equ 22

FUNC_ARGS_OFFSET  equ 24
ARG0_OFFSET equ 24 
ARG1_OFFSET equ 26 

; GDT selectors
CODE_SEL equ 0x08
DATA_SEL equ 0x10

