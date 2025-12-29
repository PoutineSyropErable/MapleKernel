// multiboot2.hpp - C++ header
#pragma once

#include <stdint.h>

namespace multiboot
{

// Tag types
enum class tag_type : uint32_t
{
	END			 = 0,
	CMDLINE		 = 1,
	BOOT_LOADER	 = 2,
	MODULE		 = 3,
	BASIC_MEM	 = 4,
	BOOTDEV		 = 5,
	MMAP		 = 6,
	VBE			 = 7,
	FRAMEBUFFER	 = 8,
	ELF_SECTIONS = 9,
	APM			 = 10,
	EFI32		 = 11,
	EFI64		 = 12,
	SMBIOS		 = 13,
	ACPI_OLD	 = 14, // ACPI 1.0 RSDP
	ACPI_NEW	 = 15, // ACPI 2.0+ RSDP
	NETWORK		 = 16,
	EFI_MMAP	 = 17,
	EFI_BS		 = 18,
	EFI32_IH	 = 19,
	EFI64_IH	 = 20,
	LAST		 = 20,
};

// Packed structures
struct __attribute__((packed)) tag
{
	enum tag_type type;
	uint32_t	  size;
	// char     data[];
};

// Array of tag names (must be in same order as enum!)
static const char *const TAG_NAMES[] = {
	"END",			// 0
	"CMDLINE",		// 1
	"BOOT_LOADER",	// 2
	"MODULE",		// 3
	"BASIC_MEM",	// 4
	"BOOTDEV",		// 5
	"MMAP",			// 6
	"VBE",			// 7
	"FRAMEBUFFER",	// 8
	"ELF_SECTIONS", // 9
	"APM",			// 10
	"EFI32",		// 11
	"EFI64",		// 12
	"SMBIOS",		// 13
	"ACPI_OLD",		// 14
	"ACPI_NEW",		// 15
	"NETWORK",		// 16
	"EFI_MMAP",		// 17
	"EFI_BS",		// 18
	"EFI32_IH",		// 19
	"EFI64_IH"		// 20
};

inline const char *type_to_str(tag_type type)
{
	uint32_t idx = static_cast<uint32_t>(type); // Convert first
	if (idx < sizeof(TAG_NAMES) / sizeof(TAG_NAMES[0]))
	{
		return TAG_NAMES[idx];
	}
	return "UNKNOWN_TAG";
}

struct multiboot_tag_old_acpi
{
	uint32_t type;
	uint32_t size;
	uint8_t	 rsdp[0];
};

struct multiboot_tag_new_acpi
{
	uint32_t type;
	uint32_t size;
	uint8_t	 rsdp[0];
};

struct module
{
	uint32_t mod_start;
	uint32_t mod_end;
	char	 cmdline[0];
};

struct module_and_len
{
	uint32_t	  cmd_len;
	bool		  none;
	struct module mod;
};

struct __attribute__((packed)) info
{
	uint32_t total_size;
	uint32_t reserved;

	// tags follow immediately in memory
};

// Helper functions
inline tag *tag_next(tag *t)
{
	if (!t)
		return nullptr;

	// Calculate next tag with 8-byte alignment
	uintptr_t addr = reinterpret_cast<uintptr_t>(t);
	uintptr_t next = addr + ((t->size + 7) & ~7UL);

	return reinterpret_cast<tag *>(next);
}

inline tag *tag_first(info *mbi)
{
	if (!mbi)
		return nullptr;
	return reinterpret_cast<tag *>(reinterpret_cast<uintptr_t>(mbi) + sizeof(uint32_t) + sizeof(uint32_t));
}

enum class rsdp_type : uint8_t
{
	NULL_V0 = 0,
	OLD_V1	= 1,
	NEW_V2	= 2,

};

struct rsdp_tagged_t
{
	enum rsdp_type new_or_old;
	void		  *rsdp;
};

// Main RSDP finding function
struct rsdp_tagged_t find_rsdp(info *mbi);

// RSDP validation
bool validate_rsdp(void *rsdp_ptr);

} // namespace multiboot
