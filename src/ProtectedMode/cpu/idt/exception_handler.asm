BITS 32
extern exception_handler
extern kprintf_argc
extern PIC_sendEOI 

section .rodata
argc dd 2
NEWLINE equ 10
interrupt_printf_fmt db "In Interrupt handler: %d", NEWLINE, NEWLINE, 0
interrupt_69_fmt db NEWLINE, "Hello from Inside the Interrupt handler %d, nice!", NEWLINE, NEWLINE, 0

section .text


hlt_loop:
	mov eax, 0x1234abcd
	cli
	hlt
	jmp hlt_loop



; if writing for 64-bit, use iretq instead
%macro isr_stub 1
isr_stub_%+%1:
	push ebp
	mov ebp, esp



	push %1 
	push interrupt_printf_fmt  
	push [argc] 
	call kprintf_argc
	; add esp, 12 ; needed if i don't do the prologue and epilogue


	; cli 
	hlt_loop_%1:
	jmp hlt_loop_%1

	mov esp, ebp
	pop ebp
	; mov eax, 1
	iret
%endmacro

isr_stub 0
isr_stub 1
isr_stub 2
isr_stub 3
isr_stub 4
isr_stub 5
isr_stub 6
isr_stub 7
isr_stub 8
isr_stub 9
isr_stub 10
isr_stub 11
isr_stub 12
isr_stub 13
isr_stub 14
isr_stub 15
isr_stub 16
isr_stub 17
isr_stub 18
isr_stub 19
isr_stub 20
isr_stub 21
isr_stub 22
isr_stub 23
isr_stub 24
isr_stub 25
isr_stub 26
isr_stub 27
isr_stub 28
isr_stub 29
isr_stub 30
isr_stub 31



isr_stub 32
isr_stub 33
isr_stub 34
isr_stub 35
isr_stub 36
isr_stub 37
isr_stub 38
isr_stub 39
isr_stub 40
isr_stub 41
isr_stub 42
isr_stub 43
isr_stub 44
isr_stub 45
isr_stub 46
isr_stub 47
isr_stub 48



global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    49 
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep


global interrupt_34_handler 
interrupt_34_handler: 
	push ebp
	mov ebp, esp

	push 34 
	push interrupt_printf_fmt  
	push [argc] 
	call kprintf_argc
	add esp, 12

	mov esp, ebp
	pop ebp
	; mov eax, 1
	iret


global interrupt_69_handler 
interrupt_69_handler: 
	push ebp
	mov ebp, esp

	push 69
	push interrupt_69_fmt
	push [argc] 
	call kprintf_argc
	add esp, 12

	mov esp, ebp
	pop ebp
	; mov eax, 1
	iret





interrupt_5_fmt db "INT 5: BOUND Range Exceeded", NEWLINE, 0
err_inf_fmt_5 db "EIP = 0x%x, CS = 0x%x, EFLAGS = 0x%x", NEWLINE, 0

global interrupt_5_handler 
interrupt_5_handler: 
    push ebp
    mov ebp, esp

    ; Save all registers
    pusha 
    push ds 
    push es 
    push fs 
    push gs 

    ; Load kernel data segments
    mov ax, 0x10  ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Print interrupt message
    push interrupt_5_fmt
    push 1
    call kprintf_argc
    add esp, 8

    ; Get interrupt frame values
    ; NO ERROR CODE FOR INT 5!
    mov ebx, [ebp + 4]   ; EIP (return address)
    mov ecx, 0
    mov cx, word [ebp + 8]  ; CS
    mov edx, [ebp + 12]     ; EFLAGS

    ; Print debug info
    push edx
    push ecx
    push ebx
    push err_inf_fmt_5
    push 4  ; 3 format specifiers + 1
    call kprintf_argc
    add esp, 20

    ; Optional: dump stack to debug corruption
    ; call dump_stack_trace

    ; For debugging: halt instead of returning
    ; If this is a serious error, panic
    ; call kernel_panic

	hll5: 
	jmp hll5

    ; Restore registers
    pop gs 
    pop fs 
    pop es
    pop ds 
    popa

    ; Return from interrupt
    mov esp, ebp
    pop ebp
    iret

; ==========================


interrupt_8_fmt db "In Interrupt handler: %d, DOUBLE_FAULT", NEWLINE, 0
err_inf_fmt db "Error Code = %u, EIP = %h, CS = %u, Eflags = %b" , NEWLINE, 0

global interrupt_8_handler 
interrupt_8_handler: 
	push ebp
	mov ebp, esp

	pusha 
	push ds 
	push es 
	push fs 
	push gs 




	push 8 ; %d
	push interrupt_8_fmt ; fmt
	push 2 ; argc
	call kprintf_argc
	add esp, 12


ERROR_CODE_Of equ 0
EIP_Of equ 4 
CS_Of equ 8 
EFLAGS_Of equ 12
	;; argc = number of % + 1; 
	mov eax, [ebp + ERROR_CODE_Of]   ; error code
	mov ebx, [ebp + EIP_Of]  ; EIP 
	mov ecx, 0
	mov cx, word [ebp + CS_Of] ; CS
	mov edx, [ebp + EFLAGS_Of] ; Eflags


	push edx
	push ecx
	push ebx
	push eax
	push err_inf_fmt 
	push 5 ; argc 
	call kprintf_argc
	add esp, 24


	pop gs 
	pop fs 
	pop es
	pop ds 
	popa


	mov esp, ebp
	pop ebp
	iret



interrupt_13_fmt db "In Interrupt handler: %d, GENERAL PROTECTION FAULT", NEWLINE, 0

global interrupt_13_handler 
interrupt_13_handler: 
	push ebp
	mov ebp, esp

	pusha 
	push ds 
	push es 
	push fs 
	push gs 




	push 13 ; %d
	push interrupt_13_fmt ; fmt
	push 2 ; argc
	call kprintf_argc
	add esp, 12


ERROR_CODE_Of3 equ 4
EIP_Of3 equ 8 
CS_Of3 equ 12 
EFLAGS_Of3 equ 16
	;; argc = number of % + 1; 
	mov eax, [ebp + ERROR_CODE_Of3]   ; error code
	mov ebx, [ebp + EIP_Of3]  ; EIP 
	mov ecx, 0
	mov cx, word [ebp + CS_Of3] ; CS
	mov edx, [ebp + EFLAGS_Of3] ; Eflags


	push edx
	push ecx
	push ebx
	push eax
	push err_inf_fmt 
	push 5 ; argc 
	call kprintf_argc
	add esp, 24




%define hardware_breakpoint 
%define software_breakpoint

%ifdef hardware_breakpoint
    mov eax, [ebp + EIP_Of3]
    mov dr0, eax
    
    ; Set DR7 for instruction execution breakpoint on DR0
    ; L0=1 (enable), G0=0, LEN0=00 (1-byte), R/W0=00 (execute only)
    mov eax, dr7
    and eax, ~0x000F000F  ; Clear DR0 settings
    or eax, 0x00000001    ; Set L0=1
    mov dr7, eax
%endif


	pop gs 
	pop fs 
	pop es
	pop ds 
	popa

	mov esp, ebp
	pop ebp
	iret




interrupt_11_fmt db "In Interrupt handler: %d, RESERVED PCI", NEWLINE, 0
outb_err_fmt db "Outb error: al = %h, dx = %h", NEWLINE , 0
global interrupt_11_handler 
interrupt_11_handler: 
	push ebp
	mov ebp, esp

	pusha 
	push ds 
	push es 
	push fs 
	push gs 


	mov ah, 0
	push dx 
	push ax 
	push outb_err_fmt
	push 3 
	call kprintf_argc 
	add esp, 16



	push 11 ; %d
	push interrupt_11_fmt ; fmt
	push 2 ; argc
	call kprintf_argc
	add esp, 12


ERROR_CODE_Of3 equ 4
EIP_Of3 equ 8 
CS_Of3 equ 12 
EFLAGS_Of3 equ 16
	;; argc = number of % + 1; 
	mov eax, [ebp + ERROR_CODE_Of3]   ; error code
	mov ebx, [ebp + EIP_Of3]  ; EIP 
	mov ecx, 0
	mov cx, word [ebp + CS_Of3] ; CS
	mov edx, [ebp + EFLAGS_Of3] ; Eflags


	push edx
	push ecx
	push ebx
	push eax
	push err_inf_fmt 
	push 5 ; argc 
	call kprintf_argc
	add esp, 24



	cli
hl11: 
	jmp hl11


	pop gs 
	pop fs 
	pop es
	pop ds 
	popa


	mov esp, ebp
	pop ebp
	iret



global interrupt_56_handler
interrupt_56_handler:
	push ebp
	mov ebp, esp

	push 56
	push interrupt_printf_fmt  
	push [argc] 
	call kprintf_argc
	; add esp, 12 ; needed if i don't do the prologue and epilogue

	mov esp, ebp
	pop ebp
	; mov eax, 1
	iret



interrupt_57_fmt db "Core %u Recieved interrupt entered main", NEWLINE, 0
interrupt_57_fmt1 db "(In interrupt handler) Recieved interrupt entered main", NEWLINE, 0

global interrupt_57_handler
interrupt_57_handler:
	push ebp
	mov ebp, esp

	; push 57 
	; push interrupt_printf_fmt  
	; push [argc] 
	; call kprintf_argc
	; add esp, 12 ; needed if i don't do the prologue and epilogue
	
	mov eax, fs:[0]

	; push eax
	push interrupt_57_fmt1
	push 1 
	call kprintf_argc

	mov dword [0xFEE00B0], 0

	mov esp, ebp
	pop ebp
	; mov eax, 1
	iret


section .bss
saved_al resb 1      ; 1 byte for AL
saved_dx resw 1      ; 2 bytes for DX




