// kernel_cpp.cpp
#include "kernel_cpp.h"
#include "kernel_cpp.hpp"
#include "stdio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// #include "framebuffer.h"
#include "acpi.hpp"
#include "apic.hpp"
#include "assert.h"
#include "framebuffer.hpp"
#include "madt.hpp"
#include "pit.hpp"
#include "ps2_mouse_handler.h"

#include "gdt.h"
#include "multicore_gdt.hpp"
#include "pic_public.h"
#include "string.h"

#include "special_pointers.hpp"

void print_test()
{
	// -O1 Cpp is wack
	kprintf("Number With no option: %u\n", 0x24f);
	char s[6] = {'a', 'b', 'c', 'd', 'e', 'f'};
	kprintf("string with option: %s;5\n", s);
	kprintf("Number with option: %u:7\n", 0x24f);
}

void multicore_setup(void *rsdp_void)
{

	struct apic::apic_support apic_support = apic::has_apic();
	kprintf("Support:\n  Apic: %b\n  x2Apic: %b\n\n\n", apic_support.apic, apic_support.x2apic);

	bool has_apic = apic_support.apic || apic_support.x2apic;
	assert(has_apic, "Must have apic\n");

	struct acpi::RSDP *rsdp = (struct acpi::RSDP *)rsdp_void;
	kprintf("rsdp address: %h\n", rsdp);
	acpi::print_rsdp(rsdp);

	acpi::RSDT *rsdt = rsdp->RsdtAddress;
	kprintf("rsdt address: %h\n", rsdt);
	acpi::print_rsdt(rsdt);

	acpi::madt::MADT *madt = acpi::findMADT(rsdt);
	kprintf("madt = %h\n", madt);
	acpi::madt::print_madt(madt);

	struct acpi::madt::MADTParseResult parsed_madt = acpi::madt::parse_madt(madt);
	kprintf("\n\n\n\n======================\n\n\n\n");
	acpi::madt::print_parsed_madt(parsed_madt);

	void *lapic_address	  = (void *)madt->lapic_address; // The main one
	void *io_apic_address = (void *)parsed_madt.io_apics[0]->io_apic_address;

	kprintf("lapic_address = %h\n", lapic_address);
	kprintf("io_apic_address = %h\n", io_apic_address);

	apic::lapic_address	   = (volatile void *)lapic_address;
	apic::io_appic_address = (volatile void *)io_apic_address;

	uint8_t core_count = parsed_madt.entry_counts.processor_local_apic;

	apic::init_apic();
	apic::init_lapic();

#define BOOT_CORE_ID 0
	uint8_t core_id = BOOT_CORE_ID;
	multicore_gdt::init_multicore_gdt();
	multicore_gdt::set_fs_or_segment_selector(core_id, multicore_gdt::fs_or_gs::fs); // sets fs so it has the correct gdt entry.
	multicore_gdt::set_fs_or_segment_selector(core_id, multicore_gdt::fs_or_gs::gs);
	multicore_gdt::get_fs_struct()->core_id		= core_id;
	multicore_gdt::get_gs_struct()->other_stuff = 69;

	if (false)
	{
		uint8_t local_core_id = apic::get_core_id();
		// Other cores will do this using get_core id;

		multicore_gdt::set_fs_or_segment_selector(local_core_id, multicore_gdt::fs_or_gs::fs); // sets fs so it has the correct gdt entry.
		multicore_gdt::set_fs_or_segment_selector(local_core_id, multicore_gdt::fs_or_gs::gs);
		multicore_gdt::get_fs_struct()->core_id		= local_core_id;
		multicore_gdt::get_gs_struct()->other_stuff = 69;
	}

	// initiate the apic_io of this core.
	apic::init_io_apic();

	// Apic timers calibration using pit
	apic::calibrate_lapic_timer();

	// wake cores. We are already on core 0.
	for (uint8_t i = 0; i < core_count; i++)
	{
		uint8_t core_id = parsed_madt.processor_local_apics[i]->apic_id;
		if (core_id == BOOT_CORE_ID)
		{
			// nothing says the parse is in order. We are on core 0.
			// So, skip that itteration
			continue;
		}
		// TODO: Must implement these functions.
		// Make is so every interrupt also does a last core recieved thing
		apic::wake_core(core_id, apic::core_bootstrap, apic::core_main);
		apic::wait_till_interrupt(INTERRUPT_ENTERED_MAIN);
	}

	disable_pic();
	// Disabling the pic will be done rather late
}

int cpp_main(struct cpp_main_args args)
{

	test_special_pointers();
	disable_pic();
	// multicore_setup(args.rsdp_v);

	kprintf("\n\n================= Start of CPP Main =================\n\n");

	// Setup lapic irq handling
	terminal_writestring("\n====kernel cpp entering main loop====\n");
	while (true)
	{
		// kernel main loop
		cpp_event_loop();

		pit::wait(1.f / 60.f);
	}
	return 0;
}

int cpp_event_loop(void)
{
	static int16_t			  old_x = 0;
	static int16_t			  old_y = 0;
	static framebuffer::Color old_color(0xff0000);
	static framebuffer::Color current_color(0x00ff00);
	static bool				  first_itt = true;

	// The order is important
	if (!first_itt)
		framebuffer::g_framebuffer.set_pixel(old_x, old_y, old_color);
	current_color = framebuffer::g_framebuffer.get_pixel(g_mouse_prop.x, g_mouse_prop.y);
	// current_color.print();
	framebuffer::g_framebuffer.set_pixel(g_mouse_prop.x, g_mouse_prop.y, framebuffer::Color(0xffffff));

	old_x	  = g_mouse_prop.x;
	old_y	  = g_mouse_prop.y;
	old_color = current_color;
	first_itt = false;
	return 0;
}
