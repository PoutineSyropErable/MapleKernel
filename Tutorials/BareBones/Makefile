# -----------------------
# Makefile for MyOS
# -----------------------

# Tools
CC32       := i686-elf-gcc
CC16       := ia16-elf-gcc
NASM       := nasm
GRUBFILE   := grub-file
GRUBMKRESCUE := grub-mkrescue
QEMU       := qemu-system-i386

# Directories
BUILD_DIR := build
ISO_DIR   := isodir

# Sources
C32_SOURCES := kernel.c virtual_memory.c idt.c string_helper.c vga_terminal.c pit_timer.c \
               f1_binary_operation.c f2_string.c f3_segment_descriptor_internals.c \
               push_var_args.c

C16_SOURCES := realmode_functions.c

ASM32_SOURCES := call_realmode_function_wrapper32.asm
ASM16_SOURCES := call_realmode_function_wrapper16.asm

BOOT_ASM := boot_intel.asm

# Object files
C32_OBJECTS := $(C32_SOURCES:%.c=$(BUILD_DIR)/%.o)
C16_OBJECTS := $(C16_SOURCES:%.c=$(BUILD_DIR)/%.o)
ASM32_OBJECTS := $(ASM32_SOURCES:%.asm=$(BUILD_DIR)/%.o)
ASM16_OBJECTS := $(ASM16_SOURCES:%.asm=$(BUILD_DIR)/%.o)
BOOT_OBJECT := $(BUILD_DIR)/boot.o

# Kernel binary and ISO
KERNEL_BIN := $(BUILD_DIR)/myos.bin
ISO_IMAGE  := $(BUILD_DIR)/myos.iso

# Compiler flags
CFLAGS := -std=gnu23 -ffreestanding -O2 -Wall -Wextra
CFLAGS16 := -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS := -ffreestanding -O2 -nostdlib -lgcc

# Default target
all: dirs $(KERNEL_BIN) $(ISO_IMAGE) run

# Create necessary directories
dirs:
	mkdir -p $(BUILD_DIR) $(ISO_DIR)/boot/grub

# Compile 32-bit C files
$(BUILD_DIR)/%.o: %.c | dirs
	$(CC32) -c $< -o $@ $(CFLAGS)

# Compile 16-bit C files
$(BUILD_DIR)/%.o: %.c | dirs
	$(CC16) -c $< -o $@ $(CFLAGS16)

# Assemble 32-bit ASM
$(BUILD_DIR)/%.o: %.asm | dirs
	$(NASM) -f elf32 $< -o $@

# Assemble 16-bit ASM
$(BUILD_DIR)/%.o: %.asm | dirs
	$(NASM) -f elf $< -o $@

# Assemble bootloader
$(BOOT_OBJECT): $(BOOT_ASM) | dirs
	$(NASM) -f elf32 $< -o $@

# Link kernel
$(KERNEL_BIN): $(BOOT_OBJECT) $(C32_OBJECTS) $(C16_OBJECTS) $(ASM32_OBJECTS) $(ASM16_OBJECTS)
	$(CC32) -T linker.ld -o $@ $^ $(LDFLAGS)

# Build ISO
$(ISO_IMAGE): $(KERNEL_BIN)
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/myos.bin
	cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUBMKRESCUE) -o $@ $(ISO_DIR)

# Run QEMU with ISO
run: $(ISO_IMAGE)
	@echo "Starting QEMU..."
	$(QEMU) -cdrom $(ISO_IMAGE) -no-reboot -d in_asm,int,cpu_reset -D qemu_instr.log -serial stdio & \
	QEMU_PID=$$!; \
	sleep 1; \
	vncviewer localhost:5900; \
	kill $$QEMU_PID 2>/dev/null

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)

.PHONY: all dirs run clean

