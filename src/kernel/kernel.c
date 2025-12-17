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

#ifdef QEMU
	kprintf("\n\n=========Start of Kernel Main==========\n\n");
#endif

	uint32_t cpuid_supp_test = cpuid_supported_check();
	kprintf("Cpuid Support Test (must not be 0) = %h\n", cpuid_supp_test);
	bool cpuid_supported = (cpuid_supp_test != 0);
	if (!cpuid_supported)
	{
		abort_msg("We are fucked, cpuid isn't supported. I'm not gonna support dinosaur computers\n");
		// Though, you will never see this message. The kernel will just insta stall
		// As there's nothing to print this too.
	}
	const char *vendor = get_cpuid_vendor();
	kprintf("Cpuid Vendor = %s\n", vendor);

	uint32_t max_cpuid = get_cpuid_max();
	kprintf("Max cpuid command= %h\n", max_cpuid);

	uint32_t max_extended_cpuid = get_cpuid_max_extended();
	kprintf("Max cpuid command= %h\n", max_extended_cpuid);

	const char *brand_string = get_cpuid_brand_string();
	kprintf("Brand string= %s\n", brand_string);

	struct fpu_features fpu_activated_features = init_fpu();

	if (fpu_activated_features.fpu)
	{
		float res = fpu_add(23.1, 42.6);
		kprintf("result of fpu addition: %f\n", res);
	}

#define GRUB_FRAMEBUFFER
#if !defined(GRUB_FRAMEBUFFER) || defined(QEMU)
	initialize_terminal();
	terminal_set_scroll(0);
	kprintf("\n===========Terminal Initialized=============\n");
#endif

	kprintf("addr = %u, magic = %h, is_proper_multiboot_32 = %u\n", mb2_info_addr, magic, is_proper_multiboot_32);

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

#	define BIOS_FRAMEBUFFER_HACK
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

#	elifdef BIOS_FRAMEBUFFER_HACK
	kprintf("in elif\n\n\n");
	uint32_t				 width		  = 1024;
	uint32_t				 height		  = 768;
	uint32_t				 pitch		  = 0;
	volatile struct color_t *base_address = (struct color_t *)0;

	uint16_t result = call_real_mode_function(add16_ref, width, height); // argc automatically calculated
	print_args16(&args16_start);
#	endif

#	define VIS_TEST
#	ifdef VIS_TEST
	do_test_c(base_address, width, height, pitch);
#	endif

#endif // DEBUG

	return;

#define GET_RSDP
#ifdef GET_RSDP
	// Eventually, learn to get rsdp from bios and uefi function calls
	struct rsdp_tagged_c rsdp_tagged	   = get_rsdp_grub(mb2_info_addr);
	char				*rsdp_type_names[] = {"NULL", "OLD", "NEW"};
	char				*name			   = rsdp_type_names[rsdp_tagged.new_or_old];
	void				*rsdp			   = rsdp_tagged.rsdp;
	kprintf("rsdp = %h, type=%s\n", rsdp, name);
#endif

	// init_paging();
	// init_page_bitmap();

	setup_interrupts(); // needed to have a working wait
	setup_keyboard();

	// wait_test();

	to_real16_test();
	test_printf2();
	test_assert(); // gd, and set to false and play with it

	zig_main();
	terminal_writestring("\n====kernel main entering loop====\n");

	cpp_main();
	module_main();
	while (true)
	{
		// kernel main loop
		cpp_event_loop();

		wait(1.f / 60.f);
	}

	return;

	// int* big_array = (int*)kmalloc(1024 * 1024 * 1024);
	// for (int i = 0; i < 100000; i++) {
	// 	big_array[i] = i;
	// }
	//
	// print_array_terminal(&term, big_array, 100000);
}
