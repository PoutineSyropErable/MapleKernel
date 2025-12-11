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
#include "symbols.h"

#include "pit.h"

GDT_ROOT *GDT16_ROOT = &GDT16_DESCRIPTOR;

extern char __kstrtab_start[];

void kernel_main(uint32_t mb2_info_addr, uint32_t magic, uint32_t is_proper_multiboot_32)
{

	// initialize_terminal();
	// terminal_set_scroll(0);
	kprintf("\n===========Terminal Initialized=============\n");

	// for (uint32_t i = 0; i < 5000; i++)
	// {
	//     kprintf("%c", __kstrtab_start[i]);
	// }

	// int res = find_string_offset(__kstrtab_start, 100000, "kernel_main");
	// kprintf("res = %d\n", res);
	// if (res > 0)
	//     kprintf("string = %s\n", &__kstrtab_start[res]);
	//
	// init_elf_symbols();
	// uintptr_t kma = find_symbol_address("kernel_main");
	// kprintf("kma2 = %u\n", kma);

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

#	define GRUB_FRAMEBUFFER
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

	/* Initialize terminal interface */

	kernel_test();
	test_printf();
	test_assert(); // gd, and set to false and play with it

	/* Some day the future, it might be important to know the state here (hence
	 * err_discard). But today is not that day*/
	idt_init();
	enum handle_ps2_setup_status err = handle_ps2_setup();
	switch (err)
	{
	case PS2_HS_ERR_one_keyboard_one_mouse:
	case PS2_HS_ERR_two_keyboard:
	case PS2_HS_ERR_two_mouse:
		// When there's two device

		break;
	case PS2_HS_ERR_unrecognized_device2:
	case PS2_HS_ERR_one_port_only:
		// When there's only device 1
		break;

	case PS2_HS_ERR_unrecognized_device1:
		// When there's only device 2

	case PS2_HS_ERR_no_port: break;
	}
	idt_finalize();

	// test_ps2_keyboard_commands();
	setup_keyboard();

	terminal_writestring("\n====kernel main entering loop====\n");

	cpp_main();
	zig_main();

	while (true)
	{
		// kernel main loop
	}

	return;

	// int* big_array = (int*)kmalloc(1024 * 1024 * 1024);
	// for (int i = 0; i < 100000; i++) {
	// 	big_array[i] = i;
	// }
	//
	// print_array_terminal(&term, big_array, 100000);
}
