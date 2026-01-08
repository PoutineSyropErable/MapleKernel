#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace longmode_prep
{
bool does_cpu_support_longmode();
void measure_kernel32();

struct max_addr
{
	uint8_t phys_bits;		 // Bits 0-7: Maximum physical address width
	uint8_t virt_bits;		 // Bits 8-15: Maximum linear (virtual) address width
	uint8_t guest_phys_bits; // Bits 16-23: Maximum physical address width for guest (if reported)
	uint8_t reserved;		 // Bits 24-31: Reserved
};

struct max_addr get_max_cpu_address();
void			set_64bit_page_table();
int64_t			simple_page_kernel64(uint32_t phys_address, uint64_t virtual_address, uint64_t size);
void			simplest_page_kernel(uint32_t phys_address, uint64_t virtual_address, uint64_t size);

void set_gdt64();
void set_idt64();
bool test_paging();

} // namespace longmode_prep
