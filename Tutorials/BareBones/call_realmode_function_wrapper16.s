; add16_wrapper16.s
BITS 16

extern resume32 
extern args16_start
extern stack16_start
extern add16

global call_real16_function 
global pm16_to_real16


%include "asm_constants.inc"   ; <-- include your header

section .text.pm16_and_real16_wrappers
pm16_to_real16: 
	mov ax, 0x18 
	mov ds, ax 
	mov es, ax 
	mov fs, ax 
	mov gs, ax 
	mov ss, ax


    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    ; Far jump to 16-bit wrapper
	jmp far 00:call_real16_function

call_real16_function:
	; setup 16 bit data segment
	mov ax, 0
    mov ds, ax
    mov es, ax
    mov fs, ax  
    mov gs, ax

	mov ax, stack16_start
	shr ax, 4
	mov ss, ax
    mov ss, ax
    mov esp, 0x4000
    
	;================ Copying the arguments to the stack
	mov dx, sp                  ; save SP

	mov cx, [args16_start + ARGC_OFFSET]
	cmp cx, 0
	je args_done

	mov ax, ss
	mov es, ax

	; DI points to SP
	mov di, sp

	; Move SI to last argument
	lea si, [args16_start + FUNC_ARGS_OFFSET] ; source array
	shl cx, 1 ; multiply by 2. So it's byte number

	add si, cx ; args16_start + FUNC_ARGS_OFFSET + (argc-1)*2
	sub si, 2  ; arg*2 -c 

	sub sp, cx
	add sp, 2
	shr cx, 1 ; restore cx to argc
	

	; Set backward direction
	std
	rep movsw          ; copy argN down to arg0
	cld                ; restore forward direction
	

args_done:
	mov ax, [args16_start + FUNC_OFFSET]
	call ax
	mov [args16_start + RET1_OFFSET], ax
	mov sp, dx          ; rest
	; ================ Copying the arguments to the stack

; rep movsw in 16 bit real mode
; cx: Counter (Number of words to copy)
; si: Source index (offset in ds)
; ds: Segment of the source data
; di: Destination index (offset in es)
; es: Segment of the destination data



    ; --- Switch back to protected mode ---
	lgdt [args16_start + GDT_ROOT_OFFSET]
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp far 0x10:resume32



; ============= Byte <-> Hex Table ============
; 1024  = 0x0400 = 1.00 KB
; 2048  = 0x0800 = 2.00 KB
; 4096  = 0x1000 = 4.00 KB
; 8192  = 0x2000 = 8.00 KB
; 16384 = 0x4000 = 16.00 KB
; 32768 = 0x8000 = 32.00 KB
; 65535 = 0xFFFF = 64.00 KB -1 Byte
; 65536 =0x10000 = 64.00 KB


; 0x20000 = 128.00 KB
; 0x40000 = 256.00 KB
; 0x80000 = 512.00 KB
; 0xFFFFF = 1MB - 1B
; 0x100000 = 1024.00 KB
