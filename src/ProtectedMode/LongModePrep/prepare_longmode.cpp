#include "assert.h"
#include "cpuid_results.hpp"
#include "math.hpp"

#include "cpuid.hpp"
#include "gdt64.hpp"
#include "idt64.hpp"
#include "paging64.hpp"
#include "prepare_longmode.hpp"
#include "std.hpp"
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

void measure_kernel32()
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
	main_pml4.entries[0].address_mid  = pdpt_addr.address_mid; // This is just taking the address >> 12
	main_pml4.entries[0].address_high = pdpt_addr.address_high;
	kprintf("Address mid = %h\n", main_pml4.entries[0].address_mid);

	first_pdpt.entries[0].present	   = 1;
	first_pdpt.entries[0].address_mid  = pd_addr_1.address_mid;
	first_pdpt.entries[0].address_high = pd_addr_1.address_high;

	first_page_directory.entries[0].present		 = 1;
	first_page_directory.entries[0].page_size	 = 1;
	first_page_directory.entries[0].address_mid	 = 0;
	first_page_directory.entries[0].address_high = 0;

	first_page_directory.entries[1].present		= 1;
	first_page_directory.entries[1].page_size	= 1;
	first_page_directory.entries[1].address_mid = (0x1000u * 512u) >> 12; // 12, not 21.
	// The location doesnt matter for whatever page size of level
	first_page_directory.entries[1].address_high = 0;

	union
	{
		uint64_t			 raw;
		page_directory_entry pe;
	} a{.pe = first_page_directory.entries[0]};
	kprintf("page dir entries 0: %h %h\n", a.raw);
	kprintf("page dir entries 0 low: %h\n", *(uint32_t *)&first_page_directory.entries[0]);
	kprintf("page dir entries 0 high: %h\n", *((uint32_t *)&first_page_directory.entries[0] + 1));

	kprintf("page dir entries 1 low: %h\n", *(uint32_t *)&first_page_directory.entries[1]);
	kprintf("page dir entries 1 high: %h\n", *((uint32_t *)&first_page_directory.entries[1] + 1));

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
		kprintf("\n\nThe Paging information [entry 0 unless specified otherwise] \n\n");
		kprintf("cr3 = low: %h:8, high: %h:8\n", cr3_of_setup);
		kprintf("cr3: base mid = %h\n", cr3.phys_addr_pml4_base_mid);
		kprintf("\n");
		kprintf("&main_pml4 = %h\n", &main_pml4.entries[0]);
		kprintf("main_pml4: low= %h:8, high= %h:8\n", to_uint64(main_pml4.entries[0]));
		kprintf("\n");
		kprintf("&first_pdpt = %h\n", &first_pdpt.entries[0]);
		kprintf("first_pdpt: low = %h:8, high = %h:8\n", to_uint64(first_pdpt.entries[0]));
		kprintf("\n");
		kprintf("&first_page_directory = %h\n", &first_page_directory);
		kprintf("first_page_directory: low = %h:8, high = %h:8\n", to_uint64(first_page_directory.entries[0]));
		kprintf("\n");
		kprintf("&first_page_directory (entry 1) = %h\n", &first_page_directory);
		kprintf("first_page_directory (entry 1): low = %h:8, high = %h:8\n", to_uint64(first_page_directory.entries[1]));
		kprintf("\n\n");
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

	gdt64.data_segment64.type		 = 0b0010;
	gdt64.data_segment64.long_mode64 = 1;

	memset(&gdt64.tss_segment, 0, sizeof(gdt64.tss_segment));

	gdtr_64.base_address_low  = (uint32_t)&gdt64;
	gdtr_64.base_address_high = 0;
	gdtr_64.table_limit		  = sizeof(gdt64) - 1;

	kprintf("\n");
	kprintf("&gdtr64 = %h\n", &gdtr_64);
	kprintf("gdtr64 = limit = %h, addr = %h\n", gdtr_64.table_limit, gdtr_64.base_address_low);
	kprintf("\n");
}

void set_idt64()
{
	// Zero out all entries in the IDT
	memset(&idt64, 0, sizeof(idt64_simple));

	// Fill in the IDTR
	idtr_64.limit			 = sizeof(idt64_entry_t) * 256 - 1;
	idtr_64.idt_base_address = (uint64_t)&idt64;

	// Optional debug print

	kprintf("\n");
	kprintf("&idtr64 = %h\n", &idtr_64);
	kprintf("idtr64 = limit = %h, addr = %h\n", idtr_64.limit, idtr_64.idt_base_address);
	kprintf("\n");
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

struct verified_address software_transform(uint64_t virtual_address)
{
	struct verified_address ret;
	virtual_address_split	split_addr = to_split(virtual_address);

	union cr3_uts
	{
		paging64_32::cr3_t data;
		uint64_t		   raw;
	} cr3_uts_v{.raw = cr3_of_setup};
	paging64_32::cr3_t cr3		= cr3_uts_v.data;
	uint32_t		   base_mid = cr3.phys_addr_pml4_base_mid;

	uint32_t	 usable_base = base_mid & ((1 << 21) - 1);
	uint32_t	 real_addr	 = usable_base << 12;
	struct pml4 *pml4		 = (struct pml4 *)(uintptr_t)real_addr;

	if (pml4 != &main_pml4)
	{
		ret.err = 1;
		return ret;
	}

	kprintf("The pml4 index: %u\n", split_addr.pml4_index);
	pml4_entry pml4_e = pml4->entries[split_addr.pml4_index];
	if (!pml4_e.present)
	{
		ret.err = 2;
		return ret;
	}
	struct pdpt *pdpt = (struct pdpt *)(pml4_e.address_mid << 12);

	kprintf("&pml4  = %h\n", pml4);
	kprintf("&pml4_e  = %h\n", &pml4_e);
	kprintf("pml4_e.address_mid  = %h\n", pml4_e.address_mid);
	kprintf("pdpt: %h\n", pdpt);
	kprintf("main pdpt: %h\n", &first_pdpt);
	if (pdpt != &first_pdpt)
	{
		ret.err = 3;
		return ret;
	}
	pdpt_entry pdpt_e = pdpt->entries[split_addr.pdpt_index];
	if (!pdpt_e.present)
	{
		ret.err = 4;
		return ret;
	}

	if (pdpt_e.page_size)
	{
		virtual_address_split_1gb va = to_split_1gb(virtual_address);
		ret.err						 = 0;
		ret.address					 = (pdpt_e.address_mid << 12) + va.offset;
		kprintf("1gb page size\n");
		return ret;
	}

	struct page_directory	   *pd	 = (struct page_directory *)(pdpt_e.address_mid << 12);
	struct page_directory_entry pd_e = pd->entries[split_addr.pd_index];
	if (!pd_e.present)
	{
		ret.err = 5;
		return ret;
	}

	if (pd_e.page_size)
	{
		virtual_address_split_2mb va = to_split_2mb(virtual_address);
		ret.err						 = 0;
		ret.address					 = (pd_e.address_mid << 12) + va.offset;
		kprintf("2mb page size\n");
		return ret;
	}

	struct page_table	   *pt	 = (struct page_table *)(pd_e.address_mid << 12);
	struct page_table_entry pt_e = pt->entries[split_addr.pt_index];
	if (!pt_e.present)
	{
		ret.err = 6;
		return ret;
	}
	uintptr_t page_frame_base = pt_e.address_mid << 12;

	ret.address = page_frame_base;
	// ret.address = page_frame_base + split_addr.offset;
	ret.err = 0;
	return ret;
}

extern "C" void to_compatibility_mode();
extern "C" void compatibility_entry();
bool			test_paging()
{
	// struct verified_address cm = software_transform((uint64_t)&to_compatibility_mode);
	// if (cm.err)
	// {
	// 	kprintf("there was an error calculating the address\n");
	// 	return false;
	// }
	// kprintf("To compatibility mode address | virtual : %h, physical : %h\n", to_compatibility_mode, cm.address);
	//
	// struct verified_address tcm = software_transform((uint64_t)&compatibility_entry);
	// if (tcm.err)
	// {
	// 	kprintf("there was an error calculating the address\n");
	// 	return false;
	// }
	// kprintf("Compatibility entry address | virtual : %h, physical : %h\n", compatibility_entry, tcm.address);

	kprintf("\n");
	// Do the test on paper
	uint32_t				eip	  = 0x20756b;
	struct verified_address eip_t = software_transform(eip);
	if (eip_t.err)
	{
		kprintf("Error getting eip address\n");
		return false;
	}
	kprintf("eip | virtual : %h, physical : %h\n", eip, eip_t.address);
	kprintf("\n");

	// end of function
	return true;
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

		// kprintf("Virtual address: %h\n", used_virtual_address);
		// kprintf("pml4 index: %u\n", vas[i].pml4_index);
		// kprintf("pdpt index: %u\n", vas[i].pdpt_index);
		// kprintf("pd index: %u\n", vas[i].pd_index);
		// kprintf("pt index: %u\n\n", vas[i].pt_index);
	}
	return page_count;
}

} // namespace longmode_prep
