// new_gdt.cpp
#include "stdio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "assert.h"

#include "gdt.h"
#include "multicore.h"
#include "multicore_gdt.h"
#include "multicore_gdt.hpp"
#include "string.h"

using namespace multicore_gdt;

extern "C"
{
	__attribute__((section(".bss.multicore_gdt16"))) GDT_ENTRY new_gdt[4 + 2 * MAX_CORE_COUNT];
}

struct FS_GS all_fs_and_gs[MAX_CORE_COUNT];

extern "C" void init_new_gdt();
void			init_new_gdt()
{

	gdtr32_t   gdt_descriptor = get_gdt_root();
	GDT_ENTRY *old_gdt32	  = gdt_descriptor.base_address;

	uint16_t old_gdt_entry_count = (gdt_descriptor.limit + 1) / sizeof(GDT_ENTRY);

	// 1. Calculate how many entries to copy
	uint16_t entries_to_copy = old_gdt_entry_count;
	if (entries_to_copy > NEW_GDT_ENTRY_COUNT)
	{
		entries_to_copy = NEW_GDT_ENTRY_COUNT;
		abort_msg("New gdt isn't big enough to have the old gdt entry values\n");
	}

	memcpy(new_gdt, old_gdt32, sizeof(GDT_ENTRY) * entries_to_copy);

	gdtr32_t new_gdtr;
	new_gdtr.base_address = new_gdt;
	new_gdtr.limit		  = (entries_to_copy * sizeof(GDT_ENTRY)) - 1;

	kprintf("new_gdtr = (%h, %u)\n", new_gdtr.base_address, new_gdtr.limit);

	set_gdt_root(&new_gdtr);
}

void add_gdt_entry(GDT_ENTRY *entry)
{
	gdtr32_t gdtr = get_gdt_root();

	// Get current number of entries
	uint16_t current_entries = (gdtr.limit + 1) / sizeof(GDT_ENTRY);

	// Check if we have space
	if (current_entries >= NEW_GDT_ENTRY_COUNT)
	{
		abort_msg("GDT is full! Cannot add more entries.\n");
	}

	// Calculate pointer to GDT array
	GDT_ENTRY *gdt_array = (GDT_ENTRY *)gdtr.base_address;

	// Add the new entry at the end
	gdt_array[current_entries] = *entry;

	// Update limit to include the new entry
	gdtr.limit += sizeof(GDT_ENTRY);

	kprintf("Added GDT entry at index %u, new limit = %u\n", current_entries, gdtr.limit);

	// Reload GDT (if needed - but limit change might not require reload)
	set_gdt_root(&gdtr);
}

void add_fs_or_gs(void *base_address, uint32_t size)
{

	GDT_ENTRY entry;
	entry.avl  = 0;
	entry.db   = 1;
	entry.dpl  = 0;
	entry.g	   = 0;
	entry.l	   = 0;
	entry.p	   = 1;
	entry.s	   = 1; // not system == true
	entry.type = 0b0011;

	set_segment_descriptor_base(&entry, base_address);
	set_segment_descriptor_limit(&entry, size - 1);

	add_gdt_entry(&entry);
}

void add_multicore_gdt_entry()
{
	for (uint8_t i = 0; i < MAX_CORE_COUNT; i++)
	{
		add_fs_or_gs(&all_fs_and_gs[i].fs, sizeof(FS_CONTENT));
		add_fs_or_gs(&all_fs_and_gs[i].gs, sizeof(GS_CONTENT));
	}
}

extern "C"
{
	extern uint8_t __new_gdt_end; // note: type can be uint8_t
}

void multicore_gdt::init_multicore_gdt()
{
	kprintf("new gdt = %h\n", new_gdt);
	kprintf("new gdt end = %h\n", &__new_gdt_end);
	init_new_gdt();
	add_multicore_gdt_entry();
}

void multicore_gdt::set_fs_or_segment_selector(uint8_t core_id, enum fs_or_gs segment_selector)
{
	segment_selector_t seg;
	seg.rpl	  = 0;
	seg.ti	  = 0;
	seg.index = 4 + (core_id * 2) + (uint16_t)segment_selector;

	switch (segment_selector)
	{

	case fs_or_gs::fs:
	{
		__asm__ volatile("movw %0, %%gs" : : "r"(seg) : "memory");
		break;
	}
	case fs_or_gs::gs:
	{
		__asm__ volatile("movw %0, %%fs" : : "r"(seg) : "memory");
		break;
	}
	}
}
