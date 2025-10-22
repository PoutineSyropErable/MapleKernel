// some text
#pragma once

#include <stdint.h>

typedef uint8_t mem;
// Read `count` sectors from disk into memory
// disk_number: BIOS drive number (0x00 = floppy A:, 0x80 = first hard drive / USB boot)
// lba_sector: the logical block number (0-based)
// buffer: memory location to store the sectors (segment:offset in real mode)
// count: number of sectors to read
// returns: 0 on success, nonzero on error
uint8_t bios_usb_disk_read16(uint8_t disk_number, uint32_t lba_sector, void* buffer, uint8_t count);
