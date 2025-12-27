#include "apic.hpp"
#include "framebuffer.hpp"
#include "multicore.h"
#include "multicore.hpp"
#include "stdio.h"

extern "C" void application_core_main()
{

	apic::init_lapic();
	uint8_t core_id	  = apic::get_core_id();
	uint8_t core_id_f = apic::get_core_id_fast();
	// kprintf("Core %u(%u) Reporting for duty!\n", core_id_f, core_id);

	// apic::wait_till_interrupt(INTERRUPT_ENTERED_MAIN);
	// kprintf("Recieved message from 0");

	apic::error err = apic::send_ipi(0, INTERRUPT_ENTERED_MAIN);
	if ((uint8_t)err)
	{
		framebuffer::g_framebuffer.draw_rectangle(
			{.top_left_x = 0, .top_left_y = 0, .width = 1024, .height = 600, .color = framebuffer::Color(core_id * 0x11)});
	}

	while (true)
	{
	}
}

namespace multicore
{

}
