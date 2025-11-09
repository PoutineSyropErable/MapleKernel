# ===========================
#   Build configuration
# ===========================

BUILD_DIR       := build
ISO_DIR         := isodir
GRUB_DIR        := $(ISO_DIR)/boot/grub

ASM32_SOURCES   := boot_intel.asm call_realmode_function_wrapper32.s
ASM16_SOURCES   := call_realmode_function_wrapper16.s
C32_SOURCES     := kernel.c virtual_memory.c idt.c string_helper.c \
                   vga_terminal.c pit_timer.c f1_binary_operation.c \
                   f2_string.c f3_segment_descriptor_internals.c \
                   push_var_args.c
C16_SOURCES     := realmode_functions.c

C32_OBJECTS := $(C32_SOURCES:%.c=$(BUILD_DIR)/%.o)
C16_OBJECTS := $(C16_SOURCES:%.c=$(BUILD_DIR)/%.o)
ASM32_OBJECTS := $(ASM32_SOURCES:%.asm=$(BUILD_DIR)/%.o)
ASM32_OBJECTS += $(ASM32_SOURCES:%.s=$(BUILD_DIR)/%.o)
ASM16_OBJECTS := $(ASM16_SOURCES:%.s=$(BUILD_DIR)/%.o)

KERNEL_BIN      := $(BUILD_DIR)/myos.bin
ISO_IMAGE       := $(BUILD_DIR)/myos.iso

LINKER_SCRIPT   := linker.ld

CC32       := i686-elf-gcc
CC16       := ia16-elf-gcc
NASM       := nasm


# ===========================
#       Phony Targets
# ===========================

.PHONY: all clean iso run debug

all: $(KERNEL_BIN)

iso: $(ISO_IMAGE)

run: iso
	qemu-system-i386 \
		-cdrom "$(ISO_IMAGE)" \
		-no-reboot \
		-d in_asm,int,cpu_reset \
		-D qemu_instr.log \
		-serial stdio & \
	QEMU_PID=$$!; \
	sleep 1; \
	vncviewer localhost:5900; \
	kill $$QEMU_PID 2>/dev/null

clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)


# ===========================
#  Build Directory Setup
# ===========================

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(ISO_DIR):
	mkdir -p $(ISO_DIR)

$(GRUB_DIR):
	mkdir -p $(GRUB_DIR)


# ===========================
#     Assembly rules
# ===========================

# 32-bit NASM assembly
$(BUILD_DIR)/%.asm.o: %.asm | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

$(BUILD_DIR)/%.s.o: %.s | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

# 16-bit NASM assembly
$(BUILD_DIR)/call_realmode_function_wrapper16.s.o: call_realmode_function_wrapper16.s | $(BUILD_DIR)
	$(NASM) -f elf $< -o $@


# ===========================
#    C Compilation Rules
# ===========================

# 32-bit freestanding C compilation
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC32) -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# 16-bit C compilation
$(BUILD_DIR)/realmode_functions.o: realmode_functions.c | $(BUILD_DIR)
	$(CC16) -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra


# ===========================
#         Link Kernel
# ===========================

$(KERNEL_BIN): $(ASM32_OBJECTS) $(ASM16_OBJECTS) $(C32_OBJECTS) $(C16_OBJECTS)
	@printf "\n\n====== Start of Linking =====\n\n"
	$(CC32) -T $(LINKER_SCRIPT) \
		-ffreestanding -O2 -nostdlib \
		-o $@ \
		$(ASM32_OBJECTS) \
		$(ASM16_OBJECTS) \
		$(C32_OBJECTS) \
		$(C16_OBJECTS) \
		-lgcc
	@printf "\n\n====== End of Linking =====\n\n"
	grub-file --is-x86-multiboot $@ || (echo "Not multiboot"; exit 1)


# ===========================
#        Create ISO
# ===========================

$(ISO_IMAGE): $(KERNEL_BIN) $(GRUB_DIR)
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/myos.bin
	cp grub.cfg $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR)


