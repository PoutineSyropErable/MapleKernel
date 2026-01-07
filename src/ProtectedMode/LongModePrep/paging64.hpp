#pragma once
#include "static_assert.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace paging64_32
{

struct __attribute__((packed)) cr3_t
{
	uint8_t reserved : 3				 = 0;
	bool	page_level_write_through : 1 = 0;
	bool	page_level_cache_disable : 1 = 0;
	uint8_t reserved2 : 7				 = 0;
	// base low : (The low 12 bits must be 0: 4KB/Page aligned)
	uint32_t phys_addr_pml4_base_mid;  // Put addr >> 12 here
	uint16_t phys_addr_pml4_base_high; // Put addr >> 44 here

	// Note that the max phys address bit count tells you here how much you can write
	// So the full high might not be usable
};

STATIC_ASSERT(sizeof(cr3_t) == 8, "Must be 64 bit\n");

struct __attribute__((packed)) cr3_pcide_t
{
	// I'm not gonna use this one, written for clarity
	uint16_t pcid : 12;
	uint32_t phys_addr_pml4_base_low;
	uint16_t phys_addr_pml4_base_high;
};
STATIC_ASSERT(sizeof(cr3_pcide_t) == 8, "Must be 64 bit\n");

struct __attribute__((packed, aligned(8))) pml4_entry
{
	bool	 present : 1			 = 0; // bit 0         . 1 = Present. 0 = not present. For manually added page, must override
	bool	 read_write_not_ro : 1	 = 1; // bit 1         . 1 = R+W, 0 = Write only
	bool	 user_not_supervisor : 1 = 0; // bit 2         . 1 = User, 0 = kernel
	bool	 page_write_through : 1	 = 0; // bit 3         .
	bool	 page_cache_disable : 1	 = 0; // bit 4         .
	bool	 accessed : 1			 = 0; // bit 5         .
	bool	 available : 1			 = 0; // bit 6         .
	bool	 reserved : 1			 = 0; // bit 7         .
	uint8_t	 available2 : 4			 = 0; // bit [8, 11]   .
	uint32_t address_mid : 32;			  // bit [12, 43]  . Address << 12
	uint8_t	 address_high : 8;			  // bit [44, M- 1]. Address << 44. (Some part of this is reserved, check cpuid)
	uint16_t available3 : 11	 = 0;	  // Bit [52, 62]  .
	bool	 execute_disable : 1 = 0;	  // Bit 63        .
};

STATIC_ASSERT(sizeof(pml4_entry) == 8, "Must be 64 bit\n");

struct __attribute__((aligned(0x1000))) pml4
{
	pml4_entry entries[512];
};

STATIC_ASSERT(sizeof(pml4) == 0x1000, "Must be page aligned\n");

struct __attribute__((packed)) pdpt_entry
{
	bool	 present : 1			 = 0; // bit 0
	bool	 read_write_not_ro : 1	 = 1; // bit 1
	bool	 user_not_supervisor : 1 = 0; // bit 2
	bool	 page_write_through : 1	 = 0; // bit 3
	bool	 page_cache_disable : 1	 = 0; // bit 4
	bool	 accessed : 1			 = 0; // bit 5
	bool	 available : 1			 = 0; // bit 6
	bool	 page_size : 1			 = 0; // bit 7. Set to 1 for 1GB page.
	uint8_t	 available2 : 4			 = 0; // bit [8, 11]
	uint32_t address_mid : 32;			  // bit [12, 43] address << 12
	uint8_t	 address_high : 8;			  // bit [44, M- 1] address << 44. (Some part of this is reserved, check cpuid)
	uint16_t available3 : 11	 = 0;	  // Bit [52, 62]
	bool	 execute_disable : 1 = 0;	  // Bit 63
};

struct __attribute__((aligned(0x1000))) pdpt
{
	pdpt_entry entries[512];
};
STATIC_ASSERT(sizeof(pdpt) == 0x1000, "Must be page aligned\n");

struct __attribute__((packed)) page_directory_entry
{
	bool	 present : 1			 = 0; // bit 0
	bool	 read_write_not_ro : 1	 = 1; // bit 1
	bool	 user_not_supervisor : 1 = 0; // bit 2
	bool	 page_write_through : 1	 = 0; // bit 3
	bool	 page_cache_disable : 1	 = 0; // bit 4
	bool	 accessed : 1			 = 0; // bit 5
	bool	 available : 1			 = 0; // bit 6
	bool	 page_size : 1			 = 0; // bit 7. Set to 1 for 2MB page.
	uint8_t	 available2 : 4			 = 0; // bit [8, 11]
	uint32_t address_mid : 32;			  // bit [12, 43] address << 12
	uint8_t	 address_high : 8;			  // bit [44, M- 1] address << 44. (Some part of this is reserved, check cpuid)
	uint16_t available3 : 11	 = 0;	  // Bit [52, 62]
	bool	 execute_disable : 1 = 0;	  // Bit 63
};

struct __attribute__((aligned(0x1000))) page_directory
{
	page_directory_entry entries[512];
};
STATIC_ASSERT(sizeof(page_directory) == 0x1000, "Must be page aligned\n");

struct __attribute__((packed)) page_table_entry
{
	bool	 present : 1			  = 0; // bit 0
	bool	 read_write_not_ro : 1	  = 1; // bit 1
	bool	 user_not_supervisor : 1  = 0; // bit 2
	bool	 page_write_through : 1	  = 0; // bit 3
	bool	 page_cache_disable : 1	  = 0; // bit 4
	bool	 accessed : 1			  = 0; // bit 5
	bool	 dirty : 1				  = 0; // bit 6
	bool	 page_attribute_table : 1 = 0; // bit 7
	bool	 global : 1		= 0;	  // Bit 8. If 1, page not flushed from tlb when cr3 is switched (Used for the higher half kernel pages)
	uint8_t	 available2 : 3 = 0;	  // bit [9, 11]
	uint32_t address_mid : 32;		  // bit [12, 43] address << 12
	uint8_t	 address_high : 8;		  // bit [44, M- 1] address << 44. (Some part of this is reserved, check cpuid)
	uint8_t	 available3 : 7		 = 0; // Bit [52, 59]
	uint8_t	 protection_key : 4	 = 0; // Bit [59, 62]
	bool	 execute_disable : 1 = 0; // Bit 63
};

STATIC_ASSERT(sizeof(page_table_entry) == 8, "Must be 64 bits\n");

struct __attribute__((aligned(0x1000))) page_table
{
	page_table_entry entries[512];
};

STATIC_ASSERT(sizeof(page_table) == 0x1000, "Must be Page aligned \n");

struct __attribute__((packed, aligned(8))) addr64
{
	uint32_t zeroes : 12 = 0;
	uint32_t address_mid : 32;
	uint32_t address_high : 8;
	uint32_t reserved : 12 = 0;
};

STATIC_ASSERT(sizeof(addr64) == 8, "Must be 64 bits\n");

union addr64_uts
{
	addr64 data;
	struct raw
	{
		uint32_t low;
		uint32_t high;
	} raw;
};

static inline addr64 transform_address(void *addr)
{

	uint32_t		 addr_u32 = (uint32_t)addr;
	union addr64_uts a{.raw = {.low = addr_u32, .high = 0}};

	return a.data;
}

} // namespace paging64_32
