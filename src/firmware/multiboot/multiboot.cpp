// multiboot2.cpp - C++ implementation
#include "assert.h"
#include "multiboot.h"
#include "multiboot.hpp"
#include "stdio.h"
#include "string.h"
#include <stddef.h>

namespace multiboot
{

// dumb that it works that way, but hey
struct module_and_len find_module(info *mbi, uint8_t module_index)
{
	if (!mbi)
		abort_msg("null mbi\n");
	if (mbi->total_size < 8 + 8)
		abort_msg("too small mbi\n");

	tag		 *current  = tag_first(mbi);
	uintptr_t end_addr = reinterpret_cast<uintptr_t>(mbi) + mbi->total_size;

	uint8_t find_count = 0;
	while (reinterpret_cast<uintptr_t>(current) < end_addr)
	{
		if (current->type == multiboot::tag_type::END && current->size == 8)
			break;

		if (current->type == multiboot::tag_type::MODULE)
		{
			auto *mod = reinterpret_cast<module *>(reinterpret_cast<uintptr_t>(current) + sizeof(tag));
			kprintf("Found a module\n");
			kprintf("module start : %h, module end : %h\n", mod->mod_start, mod->mod_end);
			// compute cmdline length from tag size
			int cmdline_len = current->size - sizeof(tag) - sizeof(mod->mod_start) - sizeof(mod->mod_end);
			kprintf("cmd len = %u\n", cmdline_len);

			kprintf("\n");

			if (find_count == module_index)
			{

				struct module_and_len ret;
				ret.none	= false;
				ret.cmd_len = cmdline_len;
				ret.mod		= *mod;
				return ret; // return first module if no filter
			}
			find_count++;
		}

		current = tag_next(current);
	}

	struct module_and_len ret;
	ret.none = true;

	return ret;
}

// Check if RSDP has valid signature
static bool check_rsdp_signature(const void *rsdp_ptr)
{
	const char *sig = static_cast<const char *>(rsdp_ptr);

	// Check for "RSD PTR " signature
	return (sig[0] == 'R' && sig[1] == 'S' && sig[2] == 'D' && sig[3] == ' ' && sig[4] == 'P' && sig[5] == 'T' && sig[6] == 'R' &&
			sig[7] == ' ');
}

// Calculate RSDP checksum
static bool check_rsdp_checksum(const void *rsdp_ptr)
{
	const uint8_t *data = static_cast<const uint8_t *>(rsdp_ptr);

	// First, check if it's ACPI 1.0 (20 bytes) or 2.0+ (36 bytes)
	// Revision is at byte 15 (0-based)
	uint8_t revision = data[15];
	size_t	length	 = (revision == 0) ? 20 : 36;

	uint8_t sum = 0;
	for (size_t i = 0; i < length; i++)
	{
		sum += data[i];
	}

	return (sum & 0xFF) == 0;
}

// Main implementation
struct multiboot::rsdp_tagged_t find_rsdp(info *mbi)
{

	struct multiboot::rsdp_tagged_t ret;
	if (!mbi)
	{
		abort_msg("null mbi\n");
	}

	// Check if MBI size is reasonable
	if (mbi->total_size < 8 + 8)
	{ // header + minimum tag
		abort_msg("too small\n");
		abort();
	}

	tag		 *current  = tag_first(mbi);
	uintptr_t end_addr = reinterpret_cast<uintptr_t>(mbi) + mbi->total_size;

	while (reinterpret_cast<uintptr_t>(current) < end_addr)
	{

		// kprintf("Current type: %s\n", multiboot::type_to_str(current->type));
		// kprintf("Current size: %h\n", current->size);
		// Check for end tag

		kprintf("tag type %u : \n", current->type);
		if (current->type == multiboot::tag_type::END && current->size == 8)
		{
			break;
		}

		// Check for ACPI RSDP tags
		if (current->type == multiboot::tag_type::ACPI_NEW)
		{
			// RSDP data starts right after the tag header
			void *rsdp_ptr = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(current) + sizeof(tag));
			kprintf("NEW\n");

			// Quick validation
			if (check_rsdp_signature(rsdp_ptr) && check_rsdp_checksum(rsdp_ptr))
			{
				ret.rsdp	   = rsdp_ptr;
				ret.new_or_old = multiboot::rsdp_type::NEW_V2;
				return ret;
			}
		}

		// Check for ACPI RSDP tags
		else if (current->type == multiboot::tag_type::ACPI_OLD)
		{
			// RSDP data starts right after the tag header
			void *rsdp_ptr = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(current) + sizeof(tag));
			kprintf("OLD\n");

			// Quick validation
			if (check_rsdp_signature(rsdp_ptr) && check_rsdp_checksum(rsdp_ptr))
			{
				ret.rsdp	   = rsdp_ptr;
				ret.new_or_old = multiboot::rsdp_type::OLD_V1;
				return ret;
			}
		}

		current = tag_next(current);
	}

	abort_msg("Nothing found!\n");
}

struct framebuffer_info_t find_framebuffer(info *mbi)
{

	struct multiboot::rsdp_tagged_t ret;
	if (!mbi)
	{
		abort_msg("null mbi\n");
	}

	// Check if MBI size is reasonable
	if (mbi->total_size < 8 + 8)
	{ // header + minimum tag
		abort_msg("too small\n");
		abort();
	}

	tag		 *current  = tag_first(mbi);
	uintptr_t end_addr = reinterpret_cast<uintptr_t>(mbi) + mbi->total_size;

	while (reinterpret_cast<uintptr_t>(current) < end_addr)
	{

		// Check for end tag
		if (current->type == multiboot::tag_type::END && current->size == 8)
		{
			break;
		}

		// Check for ACPI RSDP tags
		if (current->type == multiboot::tag_type::FRAMEBUFFER)
		{

			framebuffer_info_t *ret_ptr = reinterpret_cast<framebuffer_info_t *>(current);
			framebuffer_info_t	ret		= *ret_ptr;

			return ret;
		}

		current = tag_next(current);
	}

	abort_msg("Nothing found!\n");
}

bool validate_rsdp(void *rsdp_ptr)
{
	if (!rsdp_ptr)
	{
		return false;
	}

	return check_rsdp_signature(rsdp_ptr) && check_rsdp_checksum(rsdp_ptr);
}

} // namespace multiboot

// C interface implementation
extern "C"
{

	struct multiboot_module_c get_module_location(uint32_t mbi_addr, uint8_t module_index)
	{
		struct multiboot::module_and_len t = multiboot::find_module(reinterpret_cast<multiboot::info *>(mbi_addr), module_index);

		struct multiboot_module_c ret;
		ret.mod_start = t.mod.mod_start;
		ret.mod_end	  = t.mod.mod_end;
		t.cmd_len	  = ret.cmd_len;

		uint8_t max_len = t.cmd_len;
		if (t.cmd_len > 100)
			max_len = 100;
		memcpy(&ret.cmdline, t.mod.cmdline, max_len);
		return ret;
	}

	struct rsdp_tagged_c get_rsdp_grub(uint32_t mbi_addr)
	{
		struct multiboot::rsdp_tagged_t t = multiboot::find_rsdp(reinterpret_cast<multiboot::info *>(mbi_addr));
		kprintf("surveid cpp");

		struct rsdp_tagged_c ret;
		ret.new_or_old = (enum rsdp_type_c)(int)t.new_or_old;
		ret.rsdp	   = t.rsdp;
		return ret;
	}

	int multiboot_validate_rsdp(void *rsdp)
	{
		return multiboot::validate_rsdp(rsdp) ? 1 : 0;
	}

	bool validate_rsdp_c(void *rsdp_ptr)
	{
		return multiboot_validate_rsdp(rsdp_ptr);
	}

	struct framebuffer_info_t get_framebuffer(uint32_t mid_addr)
	{
		multiboot::info *mbi = (multiboot::info *)mid_addr;
		return multiboot::find_framebuffer(mbi);
	}
}
