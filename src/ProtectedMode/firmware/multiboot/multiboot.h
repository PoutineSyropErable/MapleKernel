#pragma once
#include "framebuffer_multiboot.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	enum rsdp_type_c
	{
		RSDP_TYPE_NULL	 = 0,
		RSDP_TYPE_V1_OLD = 1,
		RSDP_TYPE_V2_New = 2,
	};

	struct rsdp_tagged_c
	{
		enum rsdp_type_c new_or_old;
		void			*rsdp;
	};

	struct multiboot_module_c
	{
		uint32_t mod_start;
		uint32_t mod_end;
		uint8_t	 cmd_len;
		char	 cmdline[100];
	};

	struct __attribute__((packed)) entry_point_c
	{
		uint64_t entry_virtual;
		uint64_t size;
		uint32_t entry_physical;
	};

	extern struct entry_point_c k64;

	/*
	the module index start at 0.
	*/
	struct multiboot_module_c get_module_location(uint32_t mbi_addr, uint8_t module_index);

	// void  parse_multiboot_header(uint32_t multiboot_add);
	struct rsdp_tagged_c get_rsdp_grub(uint32_t mbi);

	bool validate_rsdp_c(void *rsdp_ptr);

	struct framebuffer_info_t get_framebuffer(uint32_t mid_addr);
	void					  print_all_symbols_32bit(uint32_t module_phys_addr);
	struct entry_point_c	  get_entry_point_physical_simple(uint32_t module_phys_addr);

#ifdef __cplusplus
}
#endif
