; =====================================================
; Multiboot Header Constants
; =====================================================
%define MB2_MAGIC      0xE85250D6
%define MB2_ARCH       0              ; i386 (32-bit)
%define MB2_TAG_END    0
%define MB2_TAG_INFO_REQ 1
%define MB2_TAG_ACPI_OLD 14
%define MB2_TAG_ACPI_NEW 15

%define BOOTLOADER_MAGIC_MB1 0x2BADB002
%define BOOTLOADER_MAGIC_MB2 0x36D76289


%define MB2_TAG_FRAMEBUFFER 5   ; correct type for framebuffer request


; =====================================================
; Multiboot Header Section
; =====================================================
header_start:
    ; Multiboot2 header
    dd MB2_MAGIC          ; Magic number
    dd MB2_ARCH           ; Architecture
    dd header_end - header_start ; Header length
    dd -(MB2_MAGIC + MB2_ARCH + (header_end - header_start)) ; Checksum
    
    ; Optional: Request ACPI RSDP tags (if you want GRUB to provide them)
    dw MB2_TAG_INFO_REQ  ; Type: Information request
    dw 0                 ; Flags
    dd 16                ; Size: 16 bytes (for 2, 12 bytes for 1) || Changed when added framebuffer? Must padd upward to nearest 8x? 
    dd MB2_TAG_ACPI_OLD  ; Request ACPI v1 RSDP
    dd MB2_TAG_ACPI_NEW  ; Request ACPI v2 RSDP
	; dd 0 ; padding

%define WIDTH 1024 
%define HEIGHT 768


	
	; === Framebuffer request tag ===
	%ifdef DEBUG 
		; Don't request the framebuffer in debug mode
	%else
		%define GRUB_FRAMEBUFFER
		%ifdef GRUB_FRAMEBUFFER 
			dw MB2_TAG_FRAMEBUFFER         ; Type: framebuffer
			dw 0                  ; Flags
			dd 24                 ; Size of this tag
			dd WIDTH              ; Width (desired)
			dd HEIGHT             ; Height (desired)
			dd 32                 ; Bits per pixel
			; dd 0                  ; Framebuffer type: 0 = RGB, 1 = EGA text
			dd 0                  ; Padding/reserved
		%endif
	%endif
    
    ; Required end tag
    dw MB2_TAG_END        ; Type: End
    dw 0                  ; Flags
    dd 8                  ; Size: 8 bytes
header_end:

; =====================================================
; Stack Section (16-byte aligned, 16 KiB)
; =====================================================
section .bss
    align 16
stack_bottom:
    resb 16384              ; reserve 16 KiB
stack_top:

; =====================================================
; Kernel Entry Point
; =====================================================
section .text
global _start
_start:
    ; Set stack pointer (stack grows downward)
	mov edx, 0xdeadfac0
    mov esp, stack_top

	
	; Is it proper multiboot? (ECX = (EAX == MB2_MAGIC_EAX))
	cmp eax, BOOTLOADER_MAGIC_MB2
	sete cl 
	movzx ecx, cl



    extern kernel_main
    ; EBX = pointer to Multiboot2 info structure
	; EAX = MAGIC value
	push ecx ; (proper multiboot)
	push eax ; magic number
	push ebx ; push mb2_info_addr
    call kernel_main
	add esp, 12

    ; Infinite halt loop if kernel_main returns
halt_loop:
    cli                     ; clear interrupt flag
    hlt                     ; halt CPU
    jmp halt_loop           ; jump back to halt

; Set symbol size (optional, useful for debugging)
; size _start, . - _start
