#include "apic.hpp"
#include "atomic.h"
#include "framebuffer.hpp"
#include "idt/idt_master.h"
#include "intrinsics.h"
#include "multicore.h"
#include "multicore.hpp"
#include "stdio.h"

volatile bool multicore::entered_main[MAX_CORE_COUNT]			   = {0};
volatile bool multicore::acknowledged_entered_main[MAX_CORE_COUNT] = {0};

extern "C" void application_core_main()
{

	kprintf("First entry\n");
	apic::init_lapic();
	uint8_t core_id	  = apic::get_core_id();
	uint8_t core_id_f = apic::get_core_id_fast();
	kprintf("Core %u(%u) Reporting for duty!\n\n", core_id_f, core_id);

	uint16_t fs_value;
	__asm__ volatile("mov %%fs, %0" : "=r"(fs_value));
	kprintf("Core %u has  fs = 0x%hx\n", core_id_f, fs_value);

	multicore::entered_main[core_id_f] = true;

	apic::error err = apic::error::none;
	// err = apic::send_ipi(0, INTERRUPT_ENTERED_MAIN);
	if ((uint8_t)err)
	{
		kprintf("Core %u had an issue sending an ipi\n", core_id_f);
		framebuffer::g_framebuffer.draw_rectangle({.top_left_x = 0,
			.top_left_y										   = static_cast<uint16_t>(core_id_f * (1080u / runtime_core_count)),
			.width											   = 1024,
			.height											   = static_cast<uint16_t>(1080 / runtime_core_count),
			.color											   = framebuffer::Color(core_id * 0x11)});
	}

	idt_finalize();
	// This core must load the idt too. And do sti, so it can recieve
	kprintf("Core %u Entering main loop\n", core_id_f);
	while (true)
	{
		// kprintf("Slave CPU,  core %u\n", core_id_f);
		__hlt();
		kprintf("Core %u Recieved an interrupt\n", core_id_f);

		// Read from it's queue.
		// handle the message from the queue
	}
}
