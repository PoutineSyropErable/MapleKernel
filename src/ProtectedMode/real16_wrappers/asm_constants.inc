; asm_constants.inc
; This is your "header file" for ASM

; Offsets into args16_start
GDT_ROOT_OFFSET   	equ 0
USELESS_PAD_OFFSET 	equ 6
ESP_OFFSET        	equ 8
SS_OFFSET 			equ 12
CS_OFFSET 			equ 14


RET1_OFFSET       equ 16
RET2_OFFSET       equ 18


FUNC_OFFSET       equ 20
FUNC_CS_OFFSET    equ 22


ARGC_OFFSET       equ 24

FUNC_ARGS_OFFSET  equ 26
ARG0_OFFSET equ 26 
ARG1_OFFSET equ 28 

; GDT selectors
CODE_SEL equ 0x10
DATA_SEL equ 0x18

