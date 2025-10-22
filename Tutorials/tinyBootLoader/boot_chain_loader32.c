#include "boot_chain_loader16.h"
#include <stdint.h>
#include <stdio.h>

const volatile mem* ONE_KB = (mem*)0x400;       // 1 KB
const volatile mem* TEN_KB = (mem*)0x2800;      // 10 KB
const volatile mem* HUNDRED_KB = (mem*)0x19000; // 100 KB
const volatile mem* ONE_MB = (mem*)0x100000;    // 1 MB

void read_disk_chunks(mem* disk_location, mem* buffer_start, uint32_t total_size) {
	const uint16_t chunk_size = 10240; // 10 KB per chunk
	uint32_t offset = 0;

	while (offset < total_size) {

		// handle final partial chunk
		if (offset + current_chunk > total_size) {
			current_chunk = total_size - offset;
		}

		// call 16-bit loader to read current chunk into the buffer
		read_disk16(disk_location + offset, ONE_MB - TEN_KB, current_chunk);

		offset += current_chunk;
	}
}
