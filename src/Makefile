CC=x86_64-elf-gcc
LD=x86_64-elf-ld
CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -m64
LDFLAGS=-T linker.ld -nostdlib

all: iso/kernel.bin

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

boot_header.o: boot_header.S
	$(CC) $(CFLAGS) -c boot_header.S -o boot_header.o

kernel.bin: kernel.o boot_header.o
	$(LD) $(LDFLAGS) kernel.o boot_header.o -o kernel.bin

iso/kernel.bin: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/
	cp boot/grub.cfg iso/boot/grub/

run: all
	grub-mkrescue -o kernel.iso iso
	qemu-system-x86_64 -cdrom kernel.iso

clean:
	rm -f *.o *.bin *.iso
	rm -rf iso

