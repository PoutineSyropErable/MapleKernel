CC     = clang
LD     = ld.lld
CFLAGS = -ffreestanding -O0 -g -Wall -Wextra -m64 -target x86_64-elf -nostdlib -fno-exceptions -fno-rtti
LDFLAGS = -T linker.ld -nostdlib

KERNEL = kernel.bin
ISO    = mykernel.iso
ISO_DIR = iso

all: $(ISO)

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

boot_header.o: boot_header.S
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): kernel.o boot_header.o
	$(LD) $(LDFLAGS) -o $@ $^

$(ISO): $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/
	cp boot/grub.cfg $(ISO_DIR)/boot/grub/
	grub-mkrescue -o $@ $(ISO_DIR)

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -serial stdio &
	QEMU_PID=$!
	vncviewer localhost:5900
	kill $QEMU_PID 2>/dev/null



debug: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -serial stdio -s -S &
	QEMU_PID=$!
	vncviewer localhost:5900
	kill $QEMU_PID 2>/dev/null

clean:
	rm -f *.o *.bin *.iso
	rm -rf $(ISO_DIR)

