#include "boot_chain_loader16.h"
#include <stdint.h>

// Memory type: simple byte

// Disk read function via BIOS
// disk = BIOS drive number (0x00 = floppy A, 0x80 = first HDD/USB)
// sector = starting sector (CHS encoded)
// count = number of sectors to read
// buffer = memory location to write to (segment:offset)
void read_disk16(uint8_t disk, uint16_t cylinder, uint8_t head, uint8_t sector, uint8_t count, void* buffer) {
	// BIOS interrupt structure in C (needs inline asm)
	// int 13h, ah=0x02 -> read sectors
	// input:
	//   AL = count
	//   CH = cylinder
	//   CL = sector
	//   DH = head
	//   DL = drive number
	//   ES:BX = buffer segment:offset

	// Yes, we need to change ES. Because we 100% need a segment to change the

	__asm__ volatile(
	    "push es\n\t"           // save ES
	    "mov bx, %[offset]\n\t" // offset of buffer
	    "mov ax, %[seg]\n\t"    // segment of buffer
	    "mov es, ax\n\t"
	    "mov ah, 0x02\n\t"     // BIOS read sector function
	    "mov al, %[count]\n\t" // number of sectors
	    "mov ch, %[cylinder]\n\t"
	    "mov cl, %[sector]\n\t"
	    "mov dh, %[head]\n\t"
	    "mov dl, %[disk]\n\t"
	    "int 0x13\n\t" // call BIOS
	    "pop es\n\t"   // restore ES
	    :
	    : [seg] "r"((uint16_t)(((uintptr_t)buffer) >> 4)),
	      [offset] "r"((uint16_t)((uintptr_t)buffer & 0xF)),
	      [count] "r"(count),
	      [cylinder] "r"(cylinder),
	      [head] "r"(head),
	      [sector] "r"(sector),
	      [disk] "r"(disk)
	    : "ax", "bx", "cx", "dx", "memory");
}
