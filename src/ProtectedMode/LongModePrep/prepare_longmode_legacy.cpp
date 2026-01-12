#include "assert.h"
#include "cpuid_results.hpp"
#include "kernel64_size.hpp"
#include "math.hpp"

#include "cpuid.hpp"
#include "gdt64.hpp"
#include "idt64.hpp"
#include "multiboot.h"
#include "paging64.hpp"
#include "prepare_longmode.hpp"
#include "std.hpp"
#include <string.h>


using namespace idt64_32;
using namespace gdt64_32;
using namespace paging64_32;


extern "C" uint8_t __kernel_start;
extern "C" uint8_t __kernel_end;

uint8_t *kernel_start = &__kernel_start;
uint8_t *kernel_end	  = &__kernel_end;

extern "C" pml4			  main_pml4{};
extern "C" pdpt			  first_pdpt{};
extern "C" page_directory first_page_directory{}; // first 2mb
extern "C" page_table	  first_page_table{};
extern "C" uint64_t		  cr3_of_setup = 0;

extern "C" gdtr			gdtr_64{};
extern "C" idtr			idtr_64{};
extern "C" gdt64_simple gdt64{};
extern "C" idt64_simple idt64{};

constexpr uint64_t round_up_div(uint64_t a, uint64_t b)
{
	return (a + b - 1) / b;
}

namespace longmode_prep {


struct virtual_address_split
{
	uint64_t offset : 12;	 // bits 0-11: offset inside the 4KB page
	uint64_t pt_index : 9;	 // bits 12-20: page table index
	uint64_t pd_index : 9;	 // bits 21-29: page directory index
	uint64_t pdpt_index : 9; // bits 30-38: PDPT index
	uint64_t pml4_index : 9; // bits 39-47: pml4 index
	uint64_t reserved : 16;	 // bits 48-63: must be zero in canonical addresses
};

// For 2MB pages (Page Size=1 in Page Directory):
struct virtual_address_split_2mb
{
	uint64_t offset : 21;	 // bits 0-20: offset inside 2MB page
	uint64_t pd_index : 9;	 // bits 21-29: page directory index
	uint64_t pdpt_index : 9; // bits 30-38: PDPT index
	uint64_t pml4_index : 9; // bits 39-47: PML4 index
	uint64_t reserved : 16;	 // bits 48-63
};

// For 1GB pages (Page Size=1 in PDPT):
struct virtual_address_split_1gb
{
	uint64_t offset : 30;	 // bits 0-29: offset inside 1GB page
	uint64_t pdpt_index : 9; // bits 30-38: PDPT index
	uint64_t pml4_index : 9; // bits 39-47: PML4 index
	uint64_t reserved : 16;	 // bits 48-63
};

virtual_address_split to_split(uint64_t virtual_address)
{
	union
	{
		virtual_address_split data;
		uint64_t			  raw;
	} uts;
	uts.raw = virtual_address;
	return uts.data;
}

virtual_address_split_2mb to_split_2mb(uint64_t virtual_address)
{
	virtual_address_split_2mb result;

	result.offset	  = virtual_address & 0x1FFFFF;		  // bits 0-20
	result.pd_index	  = (virtual_address >> 21) & 0x1FF;  // bits 21-29
	result.pdpt_index = (virtual_address >> 30) & 0x1FF;  // bits 30-38
	result.pml4_index = (virtual_address >> 39) & 0x1FF;  // bits 39-47
	result.reserved	  = (virtual_address >> 48) & 0xFFFF; // bits 48-63

	return result;
}

virtual_address_split_1gb to_split_1gb(uint64_t virtual_address)
{
	virtual_address_split_1gb result;

	result.offset	  = virtual_address & 0x3FFFFFFF;	  // bits 0-29
	result.pdpt_index = (virtual_address >> 30) & 0x1FF;  // bits 30-38
	result.pml4_index = (virtual_address >> 39) & 0x1FF;  // bits 39-47
	result.reserved	  = (virtual_address >> 48) & 0xFFFF; // bits 48-63

	return result;
}

struct verified_address
{
	uint32_t address;
	int		 err;
};


constexpr uint32_t	  max_page_count = kernel64_size::PAGE_COUNT;
virtual_address_split vas[max_page_count];
page_table			  k64_page_tables[round_up_div(max_page_count, 512)];
page_directory		  k64_page_directories[round_up_div(max_page_count, 512 * 512)];
pdpt				  k64_pdpts[round_up_div(max_page_count, 512 * 512 * 512)];


struct tables_and_entries
{
	uint16_t full_tables; // rounded up number of page
	uint16_t entries;
};

constexpr tables_and_entries lround_up_div(uint16_t a, uint16_t b)
{
	uint16_t pages	 = (a + b - 1) / b;
	uint16_t entries = a % b;
	if (entries == 0 && pages > 0)
		entries = b; // last table fully used
	return {.full_tables = pages, .entries = entries};
}

void simplest_page_kernel(uint32_t physical_address, uint64_t virtual_address, uint64_t size)
{

	uint32_t page_count = max_page_count;

	tables_and_entries page_tables		   = lround_up_div(page_count, 512);
	tables_and_entries page_directories	   = lround_up_div(page_tables.full_tables, 512);
	tables_and_entries page_directory_ptrs = lround_up_div(page_directories.full_tables, 512);
	tables_and_entries pml4s			   = lround_up_div(page_directory_ptrs.full_tables, 512);

	// print results
	kprintf("Page Tables: Pages = %u, Entries in last = %u\n", page_tables.full_tables, page_tables.entries);
	kprintf("Page Directories: Pages = %u, Entries in last = %u\n", page_directories.full_tables, page_directories.entries);
	kprintf("PDPTs: Pages = %u, Entries in last = %u\n", page_directory_ptrs.full_tables, page_directory_ptrs.entries);
	kprintf("PML4s: Pages = %u, Entries in last = %u\n", pml4s.full_tables, pml4s.entries);
	assert(pml4s.full_tables == 1, "Only 1 pml4 anyway\n");

	uint64_t used_virtual_address  = virtual_address;
	uint32_t used_physical_address = physical_address;

	uint16_t pml4_entry_index_max = pml4s.entries;
	// ================= Starting the loop for the index (which table to pick)
	for (uint16_t pdpt_index = 0; pdpt_index < page_directory_ptrs.full_tables; pdpt_index++)
	{
		bool	 last_pdpt			  = (pdpt_index == page_directory_ptrs.full_tables - 1);
		uint16_t pdpt_entry_index_max = last_pdpt ? page_directory_ptrs.entries : 512;

		for (uint16_t pd_index = 0; pd_index < page_directories.full_tables; pd_index++)
		{
			bool	 last_pd			= (pd_index == page_directories.full_tables - 1);
			uint16_t pd_entry_index_max = last_pd ? page_directories.entries : 512;

			for (uint16_t pt_index = 0; pt_index < page_tables.full_tables; pt_index++)
			{
				bool	 last_pt			= (pt_index == page_tables.full_tables - 1);
				uint16_t pt_entry_index_max = last_pt ? page_tables.entries : 512;

				// ================= Starting the loop for the index of the entries inside a table
				virtual_address_split va = to_split(used_virtual_address);
				addr64				  pa = transform_address((void *)used_physical_address);

				pdpt		   *pdpt = &k64_pdpts[pdpt_index];
				page_directory *pd	 = &k64_page_directories[pd_index];
				page_table	   *pt	 = &k64_page_tables[pt_index];

				for (uint16_t pml4_entry_index = 0; pml4_entry_index < pml4_entry_index_max; pml4_entry_index++)
				{
					pml4_entry *pml4_e = &main_pml4.entries[va.pml4_index];
					if (!pml4_e->present)
					{
						pml4_e->present		 = 1;
						addr64 pdpt_addr	 = transform_address(pdpt);
						pml4_e->address_mid	 = pdpt_addr.address_mid;
						pml4_e->address_high = pdpt_addr.address_high;
						pml4_e->read_write_not_ro = 1;

					}
					// End of loop
				}
				for (uint16_t pdpt_entry_index = 0; pdpt_entry_index < pdpt_entry_index_max; pdpt_entry_index++)
				{
					va				   = to_split(used_virtual_address);
					pa				   = transform_address((void *)used_physical_address);
					pdpt_entry *pdpt_e = &pdpt->entries[va.pdpt_index];
					if (!pdpt_e->present)
					{
						pdpt_e->present		 = 1;
						addr64 pd_addr		 = transform_address(pd);
						pdpt_e->address_mid	 = pd_addr.address_mid;
						pdpt_e->address_high = pd_addr.address_high;
						pdpt_e->read_write_not_ro = 1;
					}
				}
				for (uint16_t pd_entry_index = 0; pd_entry_index < pd_entry_index_max; pd_entry_index++)
				{
					va						   = to_split(used_virtual_address);
					pa						   = transform_address((void *)used_physical_address);
					page_directory_entry *pd_e = &pd->entries[va.pd_index];
					if (!pd_e->present)
					{
						pd_e->present	   = 1;
						addr64 pt_addr	   = transform_address(pt);
						pd_e->address_mid  = pt_addr.address_mid;
						pd_e->address_high = pt_addr.address_high;
						pd_e->read_write_not_ro = 1;
					}
				}
				for (uint16_t pt_entry_index = 0; pt_entry_index < pt_entry_index_max; pt_entry_index++)
				{

					va					   = to_split(used_virtual_address);
					pa					   = transform_address((void *)used_physical_address);
					page_table_entry *pt_e = &pt->entries[va.pt_index];
					pt_e->address_mid	   = pa.address_mid;
					pt_e->address_high	   = pa.address_high;
					pt_e->global		   = 1;

					kernel64_size::region_t region_type = kernel64_size::get_region_type(used_virtual_address);
					switch (region_type)
					{
					case (kernel64_size::region_t::TEXT):
					{
						pt_e->present			= 1;
						pt_e->read_write_not_ro = 0;
						pt_e->execute_disable	= 0;
						break;
					}
					case (kernel64_size::region_t::RODATA):
					{
						pt_e->present			= 1;
						pt_e->read_write_not_ro = 0;
						pt_e->execute_disable	= 1;
						break;
					}

					case (kernel64_size::region_t::DATA):
					{
						pt_e->present			= 1;
						pt_e->read_write_not_ro = 1;
						pt_e->execute_disable	= 1;
						break;
					}

					case (kernel64_size::region_t::BSS):
					{
						pt_e->present			= 1;
						pt_e->read_write_not_ro = 1;
						pt_e->execute_disable	= 1;
						break;
					}

					case (kernel64_size::region_t::STACK):
					{
						pt_e->present			= 1;
						pt_e->read_write_not_ro = 1;
						pt_e->execute_disable	= 1;
						break;
					}

					case (kernel64_size::region_t::HEAP):
					{
						pt_e->present			= 1;
						pt_e->read_write_not_ro = 1;
						pt_e->execute_disable	= 1;
						break;
					}

					case (kernel64_size::region_t::GUARD):
					{
						pt_e->present			= 0;
						pt_e->read_write_not_ro = 0;
						pt_e->execute_disable	= 1;
						break;
					}

					case (kernel64_size::region_t::INVALID):
					{
						pt_e->present			= 0;
						pt_e->read_write_not_ro = 0;
						pt_e->execute_disable	= 1;
						break;
					}
					}

					used_virtual_address += 0x1000;
					used_physical_address += 0x1000;
				}
			}
		}
	}
}

}
