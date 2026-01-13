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

extern uint32_t stack_top;
extern uint32_t stack_bottom;
extern uint32_t __kernel_end;

uint32_t round_up_div(uint32_t top, uint32_t bot)
{
	uint32_t ret = top / bot;
	if (top % bot != 0)
	{
		ret++;
	}
	return ret;
}

struct module_kernel64
{
	// The argument to use when calling the 64 bit code trampoline
	// Use this to setup paging from C.
	uint32_t start_addr;
	uint32_t end_addr;
};

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
	kprintf("stack boundary: top = %h, bottom =%h\n", stack_top, stack_bottom);
	uint64_t lol = 0x5;

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

#define GRUB_FRAMEBUFFER
#if !defined(GRUB_FRAMEBUFFER) || defined(QEMU)
	initialize_terminal();
	terminal_set_scroll(0);
	kprintf("\n===========Terminal Initialized=============\n");
#endif

	kprintf("Brand string= %s\n", brand_string);
	kprintf("Max cpuid command= %h\n", max_cpuid);
	kprintf("Max extended cpuid command= %h\n", max_extended_cpuid);
	kprintf("Cpuid Vendor = %s\n", vendor);
	print_activated_fpu_features(fpu_activated_features);
	if (fpu_activated_features.fpu)
	{
		const float a	= 23.1F;
		const float b	= 42.6F;
		float		res = fpu_add(a, b);
		kprintf("result of fpu addition: (%f + %f) = %f\n", a, b, res);
	}

	kprintf("multiboot2 information addr = %h, magic = %h, is_proper_multiboot_32 = %u\n", mb2_info_addr, magic, is_proper_multiboot_32);
	bool is_proper_multiboot = is_proper_multiboot_32 != 0U;
	if (is_proper_multiboot)
	{
		kprintf("It's proper multiboot\n");
	}
	else
	{
		abort_msg("Not proper multiboot\n");
		// The abbort might not be needed anyway.
	}

// #ifndef DEBUG

// #	define BIOS_FRAMEBUFFER_HACK
#ifdef GRUB_FRAMEBUFFER
	struct framebuffer_info_t grub_fb_info = get_framebuffer(mb2_info_addr);

	uint8_t bpp	 = grub_fb_info.bit_per_pixel;
	bool	type = grub_fb_info.type != 0U;

	kprintf("\n\n\n");
	kprintf("FB addr: %h %h, pitch=%u, w=%u, h=%u, bpp=%u, type=%u\n", grub_fb_info.base_addr_high, grub_fb_info.base_addr_low,
		grub_fb_info.pitch, grub_fb_info.width, grub_fb_info.height, bpp, grub_fb_info.type);
	assert(bpp == 32, "Need 32 bit per pixel, else we are fucked!\n");
	assert(type == 1, "Need color support\n");
	kprintf("\n\n\n");

	uint32_t				 width		  = grub_fb_info.width;
	uint32_t				 height		  = grub_fb_info.height;
	uint32_t				 pitch		  = grub_fb_info.pitch;
	volatile struct color_t *base_address = (struct color_t *)(uintptr_t)grub_fb_info.base_addr_low;

	do_test_c(base_address, width, height, pitch);

#elifdef BIOS_FRAMEBUFFER_HACK
	kprintf("in elif\n\n\n");
	uint32_t				 width		  = 1024;
	uint32_t				 height		  = 768;
	uint32_t				 pitch		  = 0;
	volatile struct color_t *base_address = (struct color_t *)0;

	uint16_t result = call_real_mode_function(add16_ref, width, height); // argc automatically calculated
	print_args16(&args16_start);

#	define VIS_TEST
#	ifdef VIS_TEST
	do_test_c(base_address, width, height, pitch);
#	endif

#endif

	// #endif // DEBUG

#define GET_RSDP
#ifdef GET_RSDP
	// Eventually, learn to get rsdp from bios and uefi function calls
	kprintf("tryng to get rsdp\n");
	struct rsdp_tagged_c rsdp_tagged	   = get_rsdp_grub(mb2_info_addr);
	char				*rsdp_type_names[] = {"NULL", "OLD", "NEW"};
	char				*name			   = rsdp_type_names[rsdp_tagged.new_or_old];
	void				*rsdp_v			   = rsdp_tagged.rsdp;
	kprintf("rsdp = %h, type=%s\n", rsdp_v, name);
#endif

#define KERNEL64_MOD_INDEX 0
	struct multiboot_module_c mod = get_module_location(mb2_info_addr, KERNEL64_MOD_INDEX);
	kprintf("Found a module\n");
	kprintf("Mod start =%h, mod End = %h, mod cmdline=|%s|\n", mod.mod_start, mod.mod_end, mod.cmdline);

	// print_all_symbols_32bit(mod.mod_start);
	struct entry_point_c kernel64_bit = get_entry_point_physical_simple(mod.mod_start);

	uint32_t entry_physical = kernel64_bit.entry_physical;
	uint64_t entry_virtual	= kernel64_bit.entry_virtual;
	uint64_t kernel_size	= kernel64_bit.size;
	union
	{
		struct
		{
			uint32_t low;
			uint32_t high;
		} pm;
		uint64_t raw;
	} uts;

	uts.raw = entry_virtual;
	kprintf("\n\nThe kernel 64 bit (Long Mode) Entry point:\n");
	kprintf("Entry physical : %h\n", entry_physical);
	kprintf("Entry virtual : %h%h\n", uts.pm.high, uts.pm.low);
	uts.raw = kernel_size;
	kprintf("Kernel Size : %h%h\n", uts.pm.high, uts.pm.low);
	kprintf("Kernel Page Count : %h\n\n", round_up_div(kernel_size, 0x1024));
	char *ad = (char *)kernel64_bit.entry_physical;
	for (uint8_t i = 0; i < 8; i++)
	{
		kprintf("byte = %h\n", (uint8_t)ad[i]);
	}

	// exit(0);

	uint32_t *k64_s = (uint32_t *)mod.mod_start;
	uint32_t *k64_e = (uint32_t *)mod.mod_end;

	uintptr_t s = (uintptr_t)k64_s;
	uintptr_t e = (uintptr_t)k64_e;

	float sf   = (float)s / (1024 * 1024);
	float ef   = (float)e / (1024 * 1024);
	float size = (ef - sf);
	kprintf("Start = %f MB, End = %f MB, size = %f KB\n", sf, ef, size * 1024);

	kprintf("Kernel ends at : %h\n", &__kernel_end);
	kprintf("Kernel ends at : %f MB\n", ((float)(uintptr_t)&__kernel_end) / (1024.f * 1024.f));

	setup_interrupts_and_ps2(); // needed to have a working wait
	setup_keyboard();

	// wait_test();

	to_real16_test();
	test_printf2();
	test_assert(); // gd, and set to false and play with it

	module_main();
	zig_main();

	// char s[6] = {'a', 'b', 'c', 'd', 'e', 'f'};
	// kprintf("sig : %s;5\n", s);
	// kprintf("Number, %u:7", 0x24f);

	struct cpp_main_args cpp_args = {.rsdp_v = rsdp_v, .kernel64_address_information = kernel64_bit};
	int					 cpp_err  = cpp_main(cpp_args);
	if (cpp_err)
	{
		kprintf("\n\n\nMajor fucky wucky happened\n");
	}
	return;
}
