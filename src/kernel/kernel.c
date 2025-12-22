#include "kernel.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "call_real16_wrapper.h"
#include "idt_master.h"
#include "kernel_cpp.h"
#include "kernel_zig.h"
#include "os_registers.c"
#include "pic.h"
#include "realmode_functions.h"

#include "stdio.h"
#include "stdlib.h"
#include "vga_terminal.h"

#include "ps2.h"
#include "ps2_controller.h"
#include "ps2_keyboard_public.h"
#include "ps2_mouse.h"
#include "ps2_mouse_public.h"

#include "framebuffer.h"
#include "framebuffer_multiboot.h"
#include "multiboot.h"

#include "kernel_helper.h"

#include "Main.h"
#include "cpuid.h"
#include "fpu.h"
#include "pit.h"

#include "acpi.h"

GDT_ROOT *GDT16_ROOT = &GDT16_DESCRIPTOR;

float fpu_add(float a, float b)
{
	float result;
	asm volatile("fld %1;"	// Load 'a' onto the FPU stack
				 "fadd %2;" // Add 'b' to ST(0)
				 "fstp %0;" // Store the result into 'result' and pop the stack
		: "=m"(result)		// output
		: "m"(a), "m"(b)	// inputs
	);
	return result;
}

void kernel_main(uint32_t mb2_info_addr, uint32_t magic, uint32_t is_proper_multiboot_32)
{

#if defined(QEMU)
	kprintf("\n\n=========Start of Kernel Main==========\n\n");
#endif
	// init_paging();
	// init_page_bitmap();

	uint32_t cpuid_supp_test = cpuid_supported_check();
	bool	 cpuid_supported = (cpuid_supp_test != 0);
	terminal_write_hex("Cpuid Support Test (must not be 0) =\n", cpuid_supp_test);
	if (!cpuid_supported)
	{
		terminal_writestring("We are fucked, cpuid isn't supported. I'm not gonna support dinosaur computers\n");
		_abort();
		// Though, you will never see this message. The kernel will just insta stall
		// As there's nothing to print this too.
		// And the lack of sse support will cause an error in the abort_msg
	}
	const char *vendor			   = get_cpuid_vendor();
	uint32_t	max_cpuid		   = get_cpuid_max();
	uint32_t	max_extended_cpuid = get_cpuid_max_extended();
	const char *brand_string	   = get_cpuid_brand_string();

	// ========================== INITIALIZING FPU (NEEDED for fpu optimized code)=========================
	struct fpu_features fpu_activated_features = init_fpu();

// #define GRUB_FRAMEBUFFER
#if !defined(GRUB_FRAMEBUFFER) || defined(QEMU)
	initialize_terminal();
	terminal_set_scroll(0);
	kprintf("\n===========Terminal Initialized=============\n");
#endif

	kprintf("Brand string= %s\n", brand_string);
	kprintf("Max cpuid command= %h\n", max_extended_cpuid);
	kprintf("Cpuid Vendor = %s\n", vendor);
	kprintf("Max cpuid command= %h\n", max_cpuid);
	print_activated_fpu_features(fpu_activated_features);
	if (fpu_activated_features.fpu)
	{
		float a	  = 23.1;
		float b	  = 42.6;
		float res = fpu_add(a, b);
		kprintf("result of fpu addition: (%f + %f) = %f\n", a, b, res);
	}

	kprintf("multiboot2 information addr = %h, magic = %h, is_proper_multiboot_32 = %u\n", mb2_info_addr, magic, is_proper_multiboot_32);
	bool is_proper_multiboot = is_proper_multiboot_32;
	if (is_proper_multiboot)
	{
		kprintf("It's proper multiboot\n");
	}
	else
	{
		abort_msg("Not proper multiboot\n");
		// The abbort might not be needed anyway.
	}

#ifndef DEBUG

// #	define BIOS_FRAMEBUFFER_HACK
#	ifdef GRUB_FRAMEBUFFER
	struct framebuffer_info_t grub_fb_info = get_framebuffer(mb2_info_addr);

	uint8_t bpp	 = grub_fb_info.bit_per_pixel;
	bool	type = grub_fb_info.type;

	kprintf("FB addr: %h %h, pitch=%u, w=%u, h=%u, bpp=%u, type=%u\n", grub_fb_info.base_addr_high, grub_fb_info.base_addr_low,
		grub_fb_info.pitch, grub_fb_info.width, grub_fb_info.height, bpp, grub_fb_info.type);
	assert(bpp == 32, "Need 32 bit per pixel, else we are fucked!\n");
	assert(type == 1, "Need color support\n");

	uint32_t				 width		  = grub_fb_info.width;
	uint32_t				 height		  = grub_fb_info.height;
	uint32_t				 pitch		  = grub_fb_info.pitch;
	volatile struct color_t *base_address = (struct color_t *)grub_fb_info.base_addr_low;

	do_test_c(base_address, width, height, pitch);

#	elifdef BIOS_FRAMEBUFFER_HACK
	kprintf("in elif\n\n\n");
	uint32_t				 width		  = 1024;
	uint32_t				 height		  = 768;
	uint32_t				 pitch		  = 0;
	volatile struct color_t *base_address = (struct color_t *)0;

	uint16_t result = call_real_mode_function(add16_ref, width, height); // argc automatically calculated
	print_args16(&args16_start);

#		define VIS_TEST
#		ifdef VIS_TEST
	do_test_c(base_address, width, height, pitch);
#		endif

#	endif

#endif // DEBUG

#define GET_RSDP
#ifdef GET_RSDP
	// Eventually, learn to get rsdp from bios and uefi function calls
	struct rsdp_tagged_c rsdp_tagged	   = get_rsdp_grub(mb2_info_addr);
	char				*rsdp_type_names[] = {"NULL", "OLD", "NEW"};
	char				*name			   = rsdp_type_names[rsdp_tagged.new_or_old];
	void				*rsdp_v			   = rsdp_tagged.rsdp;
	kprintf("rsdp = %h, type=%s\n", rsdp_v, name);
#endif

	setup_interrupts(); // needed to have a working wait
	setup_keyboard();	// crash here

	// wait_test();

	to_real16_test();
	test_printf2();
	test_assert(); // gd, and set to false and play with it

	module_main();
	zig_main();

	// char s[6] = {'a', 'b', 'c', 'd', 'e', 'f'};
	// kprintf("sig : %s;5\n", s);
	// kprintf("Number, %u:7", 0x24f);

	struct cpp_main_args cpp_args = {.rsdp_v = rsdp_v};
	int					 cpp_err  = cpp_main(cpp_args);
	if (cpp_err)
	{
		kprintf("\n\n\nMajor fucky wucky happened\n");
	}
	return;
}
