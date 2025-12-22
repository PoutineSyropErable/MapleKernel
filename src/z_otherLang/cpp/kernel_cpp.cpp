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

#include "pic_public.h"

void print_test()
{
	// -O1 Cpp is wack
	kprintf("Number With no option: %u\n", 0x24f);
	char s[6] = {'a', 'b', 'c', 'd', 'e', 'f'};
	kprintf("string with option: %s;5\n", s);
	kprintf("Number with option: %u:7\n", 0x24f);
}

int cpp_main(struct cpp_main_args args)
{

	kprintf("\n\n================= Start of CPP Main =================\n\n");

	struct apic::apic_support apic_support = apic::has_apic();
	kprintf("Support:\n  Apic: %b\n  x2Apic: %b\n\n\n", apic_support.apic, apic_support.x2apic);

	bool has_apic = apic_support.apic || apic_support.x2apic;
	assert(has_apic, "Must have apic\n");

	struct acpi::RSDP *rsdp = (struct acpi::RSDP *)args.rsdp_v;
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

	disable_pic();
	// Disabling the pic will be done rather late

	// Setup lapic irq handling
	terminal_writestring("\n====kernel main entering loop====\n");
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
