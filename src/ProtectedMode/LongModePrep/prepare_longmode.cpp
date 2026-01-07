#include "assert.h"
#include "cpuid_results.hpp"
#include "math.hpp"

#include "cpuid.hpp"
#include "gdt64.hpp"
#include "idt64.hpp"
#include "paging64.hpp"
#include "prepare_longmode.hpp"
#include <string.h>

extern "C" uint8_t __kernel_start;
extern "C" uint8_t __kernel_end;

uint8_t *kernel_start = &__kernel_start;
uint8_t *kernel_end	  = &__kernel_end;

using namespace idt64_32;
using namespace gdt64_32;
using namespace paging64_32;

extern "C" pml4			  main_pml4{};
extern "C" pdpt			  first_pdpt{};
extern "C" page_directory first_page_directory{}; // first 2mb
extern "C" page_table	  first_page_table{};
extern "C" uint64_t		  cr3_of_setup = 0;

extern "C" gdtr			gdtr_64{};
extern "C" idtr			idtr_64{};
extern "C" gdt64_simple gdt64{};
extern "C" idt64_simple idt64{};

namespace longmode_prep
{

bool does_cpu_support_longmode()
{
	struct cpuid::cpuid_verified_result res = cpuid::call_cpuid(cpuid::CpuidFunction::Extended_Features, {.raw = 0});
	if (res.has_error())
	{
		abort_msg("Could not do that cpuid\n");
	}
	constexpr uint32_t cpuid_edx_bit_idx = 1 << 29;
	if (res.regs.edx & cpuid_edx_bit_idx)
	{
		kprintf("Supports long mode\n");
		return 1;
	}
	kprintf("Doesn't support long mode\n");
	return 0;
}

struct max_addr get_max_cpu_address()
{
	struct cpuid::cpuid_verified_result res = cpuid::call_cpuid(cpuid::CpuidFunction::PhysicalAddressInfo, {.raw = 0});
	if (res.has_error())
	{
		abort_msg("Could not do that cpuid\n");
	}

	cpuid_phys_addr_eax eax = *(cpuid_phys_addr_eax *)&res.regs.eax;
	kprintf("Phys bits: %u, virt bits: %u, guest phys bits: %u\n", eax.phys_bits, eax.virt_bits, eax.guest_phys_bits);
	// This is just cpu supported information. Not the actual ram count information
	// The actual current ram will be found once I'm in 64 bit mode

	struct max_addr ret;
	memcpy(&ret, &eax, sizeof(max_addr));
	return ret;
}

void measure_kernel()
{
	kprintf("Kernel start = %h, kernel end = %h\n", kernel_start, kernel_end);

	min(0u, (uintptr_t)kernel_start);
	max((uintptr_t)kernel_start, 0x40'0000);
	// kinda pointless, will map the first 4mb anyway
}

static inline void set_cr3_raw(uint64_t cr3)
{

	asm volatile("mov %0, %%cr3" : : "r"(cr3));
}

void set_64bit_page_table()
{

	addr64 pml4_addr = transform_address(&main_pml4);
	addr64 pdpt_addr = transform_address(&first_pdpt);
	addr64 pd_addr_1 = transform_address(&first_page_directory);

	main_pml4.entries[0].present	  = 1;
	main_pml4.entries[0].address_mid  = pdpt_addr.address_mid;
	main_pml4.entries[0].address_high = pdpt_addr.address_high;

	first_pdpt.entries[0].present	   = 1;
	first_pdpt.entries[0].address_mid  = pd_addr_1.address_mid;
	first_pdpt.entries[0].address_high = pd_addr_1.address_high;

	first_page_directory.entries[0].present		 = 1;
	first_page_directory.entries[0].page_size	 = 1;
	first_page_directory.entries[0].address_mid	 = 0;
	first_page_directory.entries[0].address_high = 0;

	first_page_directory.entries[1].present		 = 1;
	first_page_directory.entries[1].page_size	 = 1;
	first_page_directory.entries[1].address_mid	 = (0x1000u * 512u) >> 12;
	first_page_directory.entries[1].address_high = 0;

	paging64_32::cr3_t cr3{.phys_addr_pml4_base_mid = pml4_addr.address_mid, .phys_addr_pml4_base_high = 0};
	union cr3_uts
	{
		paging64_32::cr3_t data;
		uint64_t		   raw;
	} cr3_uts_v{.data = cr3};

	// This wouldn't actually be done right now
	if (false)
	{
		set_cr3_raw(cr3_uts_v.raw);
	}
	else
	{
		// Use this one later on
		cr3_of_setup = cr3_uts_v.raw;
	}
}

void set_gdt64()
{

	// 32-bit code segment
	gdt64.code_segment32.type		 = 0b1010;
	gdt64.code_segment32.long_mode64 = 0;
	gdt64.code_segment32.base_low	 = 0x0;
	gdt64.code_segment32.base_middle = 0x0;
	gdt64.code_segment32.base_high	 = 0x0;
	gdt64.code_segment32.limit_low	 = 0xFFFF;
	gdt64.code_segment32.limit_high	 = 0xF; // 4 GB limit with G=1
	gdt64.code_segment32.g			 = 1;	// Granularity: 4KB pages

	// 32-bit data segment
	gdt64.data_segment32.type		 = 0b0100;
	gdt64.data_segment32.long_mode64 = 0;
	gdt64.data_segment32.base_low	 = 0x0;
	gdt64.data_segment32.base_middle = 0x0;
	gdt64.data_segment32.base_high	 = 0x0;
	gdt64.data_segment32.limit_low	 = 0xFFFF;
	gdt64.data_segment32.limit_high	 = 0xF;
	gdt64.data_segment32.g			 = 1;

	gdt64.code_segment64.type		 = 0b1010;
	gdt64.code_segment64.long_mode64 = 1;

	gdt64.data_segment64.type		 = 0b1010;
	gdt64.data_segment64.long_mode64 = 1;

	memset(&gdt64.tss_segment, 0, sizeof(gdt64.tss_segment));

	gdtr_64.base_address_low  = (uint32_t)&gdt64;
	gdtr_64.base_address_high = 0;
	gdtr_64.table_limit		  = sizeof(gdt64) - 1;
}

void set_idt64()
{
	// Zero out all entries in the IDT
	memset(&idt64, 0, sizeof(idt64_simple));

	// Fill in the IDTR
	idtr_64.limit			 = sizeof(idt64_entry_t) * 256 - 1;
	idtr_64.idt_base_address = (uint64_t)&idt64;

	// Optional debug print
	kprintf("IDT64 base: %h, limit: %u\n", idtr_64.idt_base_address, idtr_64.limit);
}

struct virtual_address_split
{
	uint64_t offset : 12;	 // bits 0-11: offset inside the 4KB page
	uint64_t pt_index : 9;	 // bits 12-20: page table index
	uint64_t pd_index : 9;	 // bits 21-29: page directory index
	uint64_t pdpt_index : 9; // bits 30-38: PDPT index
	uint64_t pml4_index : 9; // bits 39-47: pml4 index
	uint64_t reserved : 16;	 // bits 48-63: must be zero in canonical addresses
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

constexpr uint64_t round_up_div(uint64_t a, uint64_t b)
{
	return (a + b - 1) / b;
}

constexpr uint16_t	  max_page_count = 1024;
virtual_address_split vas[max_page_count];
page_table			  k64_page_tables[round_up_div(max_page_count, 512)];
page_directory		  k64_page_directories[round_up_div(max_page_count, 512 * 512)];
pdpt				  k64_pdpts[round_up_div(max_page_count, 512 * 512 * 512)];

int64_t simple_page_kernel64(uint32_t phys_address, uint64_t virtual_address, uint64_t size)
{
	uint64_t page_count = size >> 12;
	if (size % 0x1000)
	{
		page_count++;
	}

	if (page_count > max_page_count)
	{
		kprintf("Return here, and increase the max page count, or change the abbort behavior\n");
		kprintf("Do it so that the 64 bit code properly does the virtual memory setup of itself\n");
		kprintf("Just use the int return value for error\n");
		kprintf("Maybe use an attribute so that the code related to adding stuff to new page table is within the start\n");
		// There could be difficulties since I'm doing all text, all data, all...
		// I would need to change it so the first few code that can do paging is at the start
		// The good solution will be to use multistep linking for the 64 bit code
		abort_msg("The page count is bigger then the max page count\n");
		return -1;
	}

	kprintf("Page count = %u\n", page_count);

	virtual_address_split va			   = to_split(virtual_address);
	uint16_t			  first_pml4_index = va.pml4_index;
	uint16_t			  first_pdpt_index = va.pdpt_index;
	uint16_t			  first_pd_index   = va.pd_index;
	uint16_t			  first_pt_index   = va.pt_index;

	uint16_t last_pml4_index = va.pml4_index;
	uint16_t last_pdpt_index = va.pdpt_index;
	uint16_t last_pd_index	 = va.pd_index;

	uint16_t pdpt_add = 0;
	uint16_t pd_add	  = 0;
	uint16_t pt_add	  = 0;

	for (uint64_t i = 0; i < page_count; i++)
	{
		uint64_t used_virtual_address  = virtual_address + i * 0x1000;
		uint32_t used_physical_address = phys_address + i * 0x1000;
		vas[i]						   = to_split(used_virtual_address);
		if (vas[i].pml4_index != last_pml4_index)
		{
			last_pml4_index = vas[i].pml4_index;
			pdpt_add		= vas[i].pml4_index - first_pml4_index;

			addr64 pdpt_addr								= transform_address(&k64_pdpts[pdpt_add]);
			main_pml4.entries[last_pml4_index].present		= 1;
			main_pml4.entries[last_pml4_index].address_mid	= pdpt_addr.address_mid;
			main_pml4.entries[last_pml4_index].address_high = pdpt_addr.address_high;
		}
		if (vas[i].pdpt_index != last_pdpt_index)
		{
			last_pdpt_index = vas[i].pdpt_index;
			pd_add			= vas[i].pdpt_index - first_pdpt_index;

			addr64 pd_addr											  = transform_address(&k64_page_directories[pd_add]);
			k64_pdpts[pdpt_add].entries[last_pdpt_index].present	  = 1;
			k64_pdpts[pdpt_add].entries[last_pdpt_index].address_mid  = pd_addr.address_mid;
			k64_pdpts[pdpt_add].entries[last_pdpt_index].address_high = pd_addr.address_high;
		}
		if (vas[i].pd_index != last_pd_index)
		{
			last_pd_index = vas[i].pd_index;
			pt_add		  = vas[i].pd_index - first_pd_index;

			// Take the address of the needed page table. (Each page table has 512 entry, which points to 512 page frame)
			// The page directory entry points to page table base
			addr64 pt_addr													 = transform_address(&k64_page_tables[pt_add]);
			k64_page_directories[pd_add].entries[last_pd_index].present		 = 1;
			k64_page_directories[pd_add].entries[last_pd_index].address_mid	 = pt_addr.address_mid;
			k64_page_directories[pd_add].entries[last_pd_index].address_high = pt_addr.address_high;
		}

		addr64 pt_addr												= transform_address((void *)used_physical_address);
		k64_page_tables[pd_add].entries[last_pd_index].present		= 1;
		k64_page_tables[pd_add].entries[last_pd_index].global		= 1;
		k64_page_tables[pd_add].entries[last_pd_index].address_mid	= pt_addr.address_mid;
		k64_page_tables[pd_add].entries[last_pd_index].address_high = pt_addr.address_high;

		kprintf("Virtual address: %h\n", used_virtual_address);
		kprintf("pml4 index: %u\n", vas[i].pml4_index);
		kprintf("pdpt index: %u\n", vas[i].pdpt_index);
		kprintf("pd index: %u\n", vas[i].pd_index);
		kprintf("pt index: %u\n\n", vas[i].pt_index);
	}
	return page_count;
}

} // namespace longmode_prep
