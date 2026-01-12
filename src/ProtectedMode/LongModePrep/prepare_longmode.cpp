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

constexpr uint64_t round_up_div(uint64_t a, uint64_t b)
{
	return (a + b - 1) / b;
}

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
		kprintf("plm4 not correct\n");
		return ret;
	}

	// kprintf("The pml4 index: %u\n", split_addr.pml4_index);
	pml4_entry pml4_e = pml4->entries[split_addr.pml4_index];
	if (!pml4_e.present)
	{
		ret.err = 2;
		kprintf("plm4_e not present\n");
		return ret;
	}
	struct pdpt *pdpt = (struct pdpt *)(pml4_e.address_mid << 12);

	// kprintf("&pml4  = %h\n", pml4);
	// kprintf("&pml4_e  = %h\n", &pml4_e);
	// kprintf("pml4_e.address_mid  = %h\n", pml4_e.address_mid);
	// kprintf("pdpt: %h\n", pdpt);
	// kprintf("main pdpt: %h\n", &first_pdpt);
	// if (pdpt != &first_pdpt)
	// {
	// 	ret.err = 3;
	// 	kprintf("Not first pdpt\n");
	// 	return ret;
	// }
	pdpt_entry pdpt_e = pdpt->entries[split_addr.pdpt_index];
	if (!pdpt_e.present)
	{
		ret.err = 4;
		kprintf("pdpt_e not present\n");
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
		kprintf("pd_e not present\n");
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
		kprintf("pt_e not present\n");
		return ret;
	}
	uintptr_t page_frame_base = pt_e.address_mid << 12;

	ret.address = page_frame_base;
	// ret.address = page_frame_base + split_addr.offset;
	ret.err = 0;
	return ret;
}

struct page_table_entry *get_page_table_address(uint64_t virtual_address)
{
	virtual_address_split split_addr = to_split(virtual_address);

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
		kprintf("plm4 not correct\n");
		return nullptr;
	}

	// kprintf("The pml4 index: %u\n", split_addr.pml4_index);
	pml4_entry pml4_e = pml4->entries[split_addr.pml4_index];
	if (!pml4_e.present)
	{
		kprintf("plm4_e not present\n");
		return nullptr;
	}
	struct pdpt *pdpt = (struct pdpt *)(pml4_e.address_mid << 12);

	// kprintf("&pml4  = %h\n", pml4);
	// kprintf("&pml4_e  = %h\n", &pml4_e);
	// kprintf("pml4_e.address_mid  = %h\n", pml4_e.address_mid);
	// kprintf("pdpt: %h\n", pdpt);
	// kprintf("main pdpt: %h\n", &first_pdpt);
	// if (pdpt != &first_pdpt)
	// {
	// 	ret.err = 3;
	// 	kprintf("Not first pdpt\n");
	// 	return ret;
	// }
	pdpt_entry pdpt_e = pdpt->entries[split_addr.pdpt_index];
	if (!pdpt_e.present)
	{
		kprintf("pdpt_e not present\n");
		return nullptr;
	}

	if (pdpt_e.page_size)
	{
		virtual_address_split_1gb va = to_split_1gb(virtual_address);
		kprintf("1gb page size\n");
		return nullptr;
	}

	struct page_directory	   *pd	 = (struct page_directory *)(pdpt_e.address_mid << 12);
	struct page_directory_entry pd_e = pd->entries[split_addr.pd_index];
	if (!pd_e.present)
	{
		kprintf("pd_e not present\n");
		return nullptr;
	}

	if (pd_e.page_size)
	{
		virtual_address_split_2mb va = to_split_2mb(virtual_address);
		kprintf("2mb page size\n");
		// return &pd_e;
		return nullptr;
	}

	struct page_table	   *pt	 = (struct page_table *)(pd_e.address_mid << 12);
	struct page_table_entry pt_e = pt->entries[split_addr.pt_index];
	if (!pt_e.present)
	{
		kprintf("pt_e not present\n");
		return nullptr;
	}

	return &pt->entries[split_addr.pt_index];
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

	struct verified_address k64_phys = software_transform(k64.entry_virtual);
	if (k64_phys.err)
	{
		kprintf("Error getting k64 address\n");
		exit(0);
		return false;
	}
	kprintf("k64 entry | virtual : low %h, high %h, physical : low %h\n", k64.entry_virtual, k64_phys.address);
	kprintf("The true physical address: low %h\n", k64.entry_physical);
	kprintf("\n");

	uint64_t k_start = k64.entry_virtual;

	for (uint64_t k_addr = k_start; k_addr < k_start + k64.size; k_addr += 0x1000)
	{
		struct verified_address k_addr_phys = software_transform(k_addr);
		if (k_addr_phys.err)
		{
			kprintf("Error getting the physical address of : %h\n", k_addr);
			// exit(0);
			// return false;
		}
	}

	kprintf("The final address: low: %h, high: %h\n", k_start + k64.size);

	kernel64_size::region_t text_region_type = kernel64_size::get_region_type(kernel64_size::KERNEL64_MAIN_ADDR);
	kprintf("The name of the region of kernel64_main: %s\n", kernel64_size::region_to_string(text_region_type));

	struct verified_address k_addr_phys = software_transform(kernel64_size::KERNEL64_MAIN_ADDR);
	if (k_addr_phys.err)
	{
		kprintf("Error getting the physical address of : %h\n", kernel64_size::KERNEL64_MAIN_ADDR);
		exit(0);
		return false;
	}

	uint64_t stack_addr = kernel64_size::STACK_TOP - 8;
	// uint64_t stack_addr = 0xffffffff8000c000;
	kprintf("The stack address : low %h, high %h\n", stack_addr);
	kernel64_size::region_t stack_region_type = kernel64_size::get_region_type(stack_addr);
	kprintf("The name of the region of stack top: %s\n", kernel64_size::region_to_string(stack_region_type));
	k_addr_phys = software_transform(stack_addr);
	if (k_addr_phys.err)
	{
		kprintf("Error getting the physical address of : %h\n", stack_addr);
		exit(0);
		return false;
	}

	page_table_entry *pt_e = get_page_table_address(stack_addr);
	kprintf("pt_e = low: %h:8, high : %h:8\n", pt_e);
	kprintf("pt_e = low: %b:32, high : %b:32\n", pt_e);
	kprintf("pt_e = {present = %u, execute_disable = %u, read_write_not_ro = %u}\n", pt_e->present, pt_e->execute_disable,
		pt_e->read_write_not_ro);

	kprintf("\n\n=====Rodata stuff======\n\n");

	uint64_t rodata_addr_1 = 0xffffffff80001008;
	kprintf("The rodata address 1: low %h, high %h\n", rodata_addr_1);
	kernel64_size::region_t rodata_1_region_type = kernel64_size::get_region_type(rodata_addr_1);
	kprintf("The name of the region of rodata address 1: %s\n", kernel64_size::region_to_string(rodata_1_region_type));
	k_addr_phys = software_transform(rodata_addr_1);
	if (k_addr_phys.err)
	{
		kprintf("Error getting the physical address of : %h\n", rodata_addr_1);
		exit(0);
		return false;
	}

	pt_e = get_page_table_address(rodata_addr_1);
	kprintf("pt_e = low: %h:8, high : %h:8\n", pt_e);
	kprintf("pt_e = low: %b:32, high : %b:32\n", pt_e);
	kprintf("pt_e = {present = %u, execute_disable = %u, read_write_not_ro = %u}\n", pt_e->present, pt_e->execute_disable,
		pt_e->read_write_not_ro);

	kprintf("\n");
	uint64_t rodata_addr_2 = 0xffffffff80003015;
	kprintf("The rodata address 2: low %h, high %h\n", rodata_addr_2);
	kernel64_size::region_t rodata_2_region_type = kernel64_size::get_region_type(rodata_addr_2);
	kprintf("The name of the region of rodata address 2: %s\n", kernel64_size::region_to_string(rodata_2_region_type));
	k_addr_phys = software_transform(rodata_addr_2);
	if (k_addr_phys.err)
	{
		kprintf("Error getting the physical address of : %h\n", rodata_addr_2);
		exit(0);
		return false;
	}

	pt_e = get_page_table_address(rodata_addr_2);
	kprintf("pt_e = low: %h:8, high : %h:8\n", pt_e);
	kprintf("pt_e = low: %b:32, high : %b:32\n", pt_e);
	kprintf("pt_e = {present = %u, execute_disable = %u, read_write_not_ro = %u}\n", pt_e->present, pt_e->execute_disable,
		pt_e->read_write_not_ro);

	// uint64_t				c_entry = 0xffffffff80405ff8;
	// struct verified_address c_phys	= software_transform(c_entry);
	// if (c_phys.err)
	// {
	// 	kprintf("Error getting centry address\n");
	// 	exit(0);
	// 	return false;
	// }
	// kprintf("C entry | virtual : low %h, high %h, physical : low %h\n", c_entry, c_phys.address);
	// kprintf("The true physical address: low %h\n", k64.entry_physical + (uint32_t)(c_entry - k64.entry_virtual));
	// kprintf("\n");

	// end of function
	return true;
}

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

int64_t simple_page_kernel64(uint32_t phys_address, uint64_t virtual_address, uint64_t size)
{

	uint32_t page_count = max_page_count;
	kprintf("Page count = %u\n", page_count);
	assert(size <= kernel64_size::MODULE_SIZE,
		"must be same size?  header={ low: %u high: %u } | grub={ low:%u, high:%u}\n",
		kernel64_size::MODULE_SIZE, size);
	// Maybe grub only gives one of these

	virtual_address_split va = to_split(virtual_address);

	uint16_t last_pml4_entry_index = va.pml4_index;
	uint16_t last_pdpt_entry_index = va.pdpt_index;
	uint16_t last_pd_entry_index   = va.pd_index;

	uint16_t pdpt_table_index = 0;
	uint16_t pd_table_index	  = 0;
	uint16_t pt_table_index	  = 0;

	for (uint64_t i = 0; i < page_count; i++)
	{
		uint64_t used_virtual_address  = virtual_address + i * 0x1000;
		uint64_t used_physical_address = phys_address + i * 0x1000;
		vas[i]						   = to_split(used_virtual_address);
		if (vas[i].pml4_index != last_pml4_entry_index || i == 0)
		{
			if (i != 0)
				pdpt_table_index++;

			addr64 pdpt_addr								  = transform_address(&k64_pdpts[pdpt_table_index]);
			main_pml4.entries[vas[i].pml4_index].present	  = 1;
			main_pml4.entries[vas[i].pml4_index].address_mid  = pdpt_addr.address_mid;
			main_pml4.entries[vas[i].pml4_index].address_high = pdpt_addr.address_high;

			last_pml4_entry_index = vas[i].pml4_index;
		}
		if (vas[i].pdpt_index != last_pdpt_entry_index || i == 0)
		{
			if (i != 0)
				pd_table_index++;

			addr64 pd_addr														= transform_address(&k64_page_directories[pd_table_index]);
			k64_pdpts[pdpt_table_index].entries[vas[i].pdpt_index].present		= 1;
			k64_pdpts[pdpt_table_index].entries[vas[i].pdpt_index].address_mid	= pd_addr.address_mid;
			k64_pdpts[pdpt_table_index].entries[vas[i].pdpt_index].address_high = pd_addr.address_high;

			last_pdpt_entry_index = vas[i].pdpt_index;
		}

		if (vas[i].pd_index != last_pd_entry_index || i == 0)
		{
			if (i != 0)
				pt_table_index++;
			// These value indeed need to go up, and not round back to 0

			// Take the address of the needed page table. (Each page table has 512 entry, which points to 512 page frame)
			// The page directory entry points to page table base
			addr64 pt_addr															  = transform_address(&k64_page_tables[pt_table_index]);
			k64_page_directories[pd_table_index].entries[vas[i].pd_index].present	  = 1;
			k64_page_directories[pd_table_index].entries[vas[i].pd_index].address_mid = pt_addr.address_mid;
			k64_page_directories[pd_table_index].entries[vas[i].pd_index].address_high = pt_addr.address_high;

			last_pd_entry_index = vas[i].pd_index;
		}

		struct page_table_entry *pt_e	 = &k64_page_tables[pt_table_index].entries[vas[i].pt_index];
		addr64					 pa_addr = transform_address((void *)used_physical_address);
		pt_e->address_mid				 = pa_addr.address_mid;
		pt_e->address_high				 = pa_addr.address_high;
		pt_e->global					 = 1;

		kernel64_size::region_t region_type = kernel64_size::get_region_type(used_virtual_address);
		switch (region_type)
		{
		case (kernel64_size::region_t::TEXT):
		{
			// kprintf("Address in text: low %h %h high\n", used_virtual_address);
			// kprintf("Address in CODE: low %h %h high\n", used_virtual_address);
			// kprintf("The physical address: %h\n", used_physical_address);

			pt_e->present			= 1;
			pt_e->read_write_not_ro = 0;
			pt_e->execute_disable	= 0;
			break;
		}
		case (kernel64_size::region_t::RODATA):
		{
			// kprintf("Address in RODATA: low %h %h high\n", used_virtual_address);
			// kprintf("The physical address: %h\n", used_physical_address);
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
			// kprintf("Address in STACK: low %h %h high\n", used_virtual_address);
			pt_e->present			= 1;
			pt_e->read_write_not_ro = 1;
			pt_e->execute_disable	= 1;
			// need execute disable?
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
			kprintf("An invalid region detected: low %h, high %h\n", used_virtual_address);
			pt_e->present			= 0;
			pt_e->read_write_not_ro = 0;
			pt_e->execute_disable	= 1;
			break;
		}
		}

		constexpr bool make_all_rwx = false;
		if (make_all_rwx)
		{
			pt_e->present			= 1;
			pt_e->read_write_not_ro = 1;
			pt_e->execute_disable	= 0;
		}

		// kprintf("Virtual address: %h\n", used_virtual_address);
		// kprintf("pml4 index: %u\n", vas[i].pml4_index);
		// kprintf("pdpt index: %u\n", vas[i].pdpt_index);
		// kprintf("pd index: %u\n", vas[i].pd_index);
		// kprintf("pt index: %u\n\n", vas[i].pt_index);
	}
	return page_count;
}

constexpr uint32_t framebuffer_size		  = 1920 * 1080 * sizeof(uint32_t);
constexpr uint32_t framebuffer_page_count = (framebuffer_size + 0x1000 - 1) / 0x1000;

constexpr tables_and_entries fb_page_tables_c		  = lround_up_div(framebuffer_page_count, 512);
constexpr tables_and_entries fb_page_directories_c	  = lround_up_div(fb_page_tables_c.full_tables, 512);
constexpr tables_and_entries fb_page_directory_ptrs_c = lround_up_div(fb_page_directories_c.full_tables, 512);
constexpr tables_and_entries fb_pml4s_c				  = lround_up_div(fb_page_directory_ptrs_c.full_tables, 512);

virtual_address_split fb_vas[framebuffer_page_count];
page_table			  fb_page_tables[fb_page_tables_c.full_tables];
page_directory		  fb_page_directories[fb_page_directories_c.full_tables];
pdpt				  fb_pdpts[fb_page_directory_ptrs_c.full_tables];

struct paging64_32::allocated_paging_structures frame_buffer_paging_struct = { //
	.pdpts		= fb_pdpts,
	.pd			= fb_page_directories,
	.pt			= fb_page_tables,
	.ppdt_count = fb_page_directory_ptrs_c.full_tables,
	.pd_count	= fb_page_directories_c.full_tables,
	.pt_count	= fb_page_tables_c.full_tables};

int64_t vmap_addresses(uint32_t phys_address, uint64_t virtual_address, uint64_t size,
	struct paging64_32::allocated_paging_structures paging_structures, enum paging64_32::vmap_address_type address_type)
{

	uint32_t page_count = round_up_div(size, 0x1000);
	kprintf("Page count = %u\n", page_count);
	assert(page_count <= paging_structures.pt_count * 512, "Must have alloacted enough page tables, Page count  %u, struct = %u\n",
		page_count, paging_structures.pt_count * 512);

	virtual_address_split va = to_split(virtual_address);

	uint16_t last_pml4_entry_index = va.pml4_index;
	uint16_t last_pdpt_entry_index = va.pdpt_index;
	uint16_t last_pd_entry_index   = va.pd_index;

	uint16_t pdpt_table_index = 0;
	uint16_t pd_table_index	  = 0;
	uint16_t pt_table_index	  = 0;
	// now

	for (uint64_t i = 0; i < page_count; i++)
	{
		uint64_t					 used_virtual_address  = virtual_address + i * 0x1000;
		uint32_t					 used_physical_address = phys_address + i * 0x1000;
		struct virtual_address_split virtual_address_split = to_split(used_virtual_address);
		if (virtual_address_split.pml4_index != last_pml4_entry_index || i == 0)
		{
			if (i != 0)
				pdpt_table_index++;

			addr64 pdpt_addr = transform_address(&fb_pdpts[pdpt_table_index]);
			// now
			main_pml4.entries[virtual_address_split.pml4_index].present		 = 1;
			main_pml4.entries[virtual_address_split.pml4_index].address_mid	 = pdpt_addr.address_mid;
			main_pml4.entries[virtual_address_split.pml4_index].address_high = pdpt_addr.address_high;

			last_pml4_entry_index = virtual_address_split.pml4_index;
		}
		if (virtual_address_split.pdpt_index != last_pdpt_entry_index || i == 0)
		{
			if (i != 0)
				pd_table_index++;

			addr64 pd_addr = transform_address(&fb_page_directories[pd_table_index]);
			fb_pdpts[pdpt_table_index].entries[virtual_address_split.pdpt_index].present	  = 1;
			fb_pdpts[pdpt_table_index].entries[virtual_address_split.pdpt_index].address_mid  = pd_addr.address_mid;
			fb_pdpts[pdpt_table_index].entries[virtual_address_split.pdpt_index].address_high = pd_addr.address_high;
			// now

			last_pdpt_entry_index = virtual_address_split.pdpt_index;
		}

		if (virtual_address_split.pd_index != last_pd_entry_index || i == 0)
		{
			if (i != 0)
				pt_table_index++;
			// These value indeed need to go up, and not round back to 0

			// Take the address of the needed page table. (Each page table has 512 entry, which points to 512 page frame)
			// The page directory entry points to page table base
			addr64 pt_addr = transform_address(&fb_page_tables[pt_table_index]);
			fb_page_directories[pd_table_index].entries[virtual_address_split.pd_index].present		 = 1;
			fb_page_directories[pd_table_index].entries[virtual_address_split.pd_index].address_mid	 = pt_addr.address_mid;
			fb_page_directories[pd_table_index].entries[virtual_address_split.pd_index].address_high = pt_addr.address_high;

			last_pd_entry_index = virtual_address_split.pd_index;
		}

		struct page_table_entry *pt_e	 = &fb_page_tables[pt_table_index].entries[virtual_address_split.pt_index];
		addr64					 pa_addr = transform_address((void *)used_physical_address);
		pt_e->global					 = 1;
		pt_e->address_mid				 = pa_addr.address_mid;
		pt_e->address_high				 = pa_addr.address_high;

		switch (address_type)
		{
		case (paging64_32::vmap_address_type::code):
		{
			pt_e->present			= 1;
			pt_e->read_write_not_ro = 0;
			pt_e->execute_disable	= 0;
			break;
		}
		case (paging64_32::vmap_address_type::rodata):
		{
			pt_e->present			= 1;
			pt_e->read_write_not_ro = 0;
			pt_e->execute_disable	= 1;
			break;
		}
		case (paging64_32::vmap_address_type::data):
		{
			pt_e->present			= 1;
			pt_e->read_write_not_ro = 1;
			pt_e->execute_disable	= 1;
			break;
		}
		case (paging64_32::vmap_address_type::mmio):
		{
			pt_e->present			 = 1;
			pt_e->read_write_not_ro	 = 1;
			pt_e->execute_disable	 = 1;
			pt_e->page_cache_disable = 1;
			pt_e->page_write_through = 1;
			// This needs page attribute table support
			// pt_e->page_attribute_table = 1;
			break;
		}
		case (paging64_32::vmap_address_type::unmaped):
		{
			pt_e->present = 0;
			break;
		}
		}

		// kprintf("Virtual address: %h\n", used_virtual_address);
		// kprintf("pml4 index: %u\n", vas[i].pml4_index);
		// kprintf("pdpt index: %u\n", vas[i].pdpt_index);
		// kprintf("pd index: %u\n", vas[i].pd_index);
		// kprintf("pt index: %u\n\n", vas[i].pt_index);
	}
	return page_count;
}

} // namespace longmode_prep
