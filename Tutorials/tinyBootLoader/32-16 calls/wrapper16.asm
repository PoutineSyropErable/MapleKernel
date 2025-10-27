; add16_wrapper.asm
; 32-bit protected mode → call 16-bit function → return to 32-bit PM
; NASM syntax

BITS 32

global call_add16_from_pm
extern add16_16bit   ; label of 16-bit function

;---------------------------------------
; call_add16_from_pm
; Arguments: 
;   eax = first operand (32-bit)
;   ebx = second operand (32-bit)
; Returns:
;   eax = result (lower 16 bits)
;---------------------------------------
call_add16_from_pm:
    ; Save 32-bit state
    pushad
    pushfd

    cli                     ; disable interrupts

    ; Prepare to switch to 16-bit real mode
    mov eax, cr0
    and eax, 0xFFFFFFFE      ; clear PE bit (protected mode disable)
    mov cr0, eax

    ; Far jump to 16-bit code segment to flush prefetch
    jmp 0x0000:real16_start

;---------------------------------------
; 16-bit code
BITS 16
real16_start:
    ; Setup 16-bit stack (choose free memory)
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xFFFF

    ; Move arguments from 32-bit registers (truncated to 16-bit)
    mov ax, word [esp+4]   ; first argument (from saved stack)
    mov bx, word [esp+6]   ; second argument

    ; Call 16-bit function
    call add16_16bit

    ; Result is in ax
    ; Store it somewhere the 32-bit code can read
    mov [0x8000], ax       ; temporary memory location

    ; Restore 32-bit protected mode
    cli
    lgdt [gdt32]           ; reload 32-bit GDT
    mov eax, cr0
    or eax, 1              ; set PE bit
    mov cr0, eax

    ; Far jump to flush prefetch and enter 32-bit CS
    jmp 32bit_start

;---------------------------------------
; 32-bit code again
BITS 32
32bit_start:
    ; Restore 32-bit registers
    mov ax, [0x8000]       ; get result from 16-bit code
    movzx eax, ax          ; zero-extend to 32-bit

    popfd
    popad
    ret

