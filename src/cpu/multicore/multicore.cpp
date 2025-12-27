#include "apic.hpp"
#include "multicore.h"
#include "multicore.hpp"
#include "stdio.h"

extern "C" void application_core_main()
{
	kprintf("Got to the core main\n");

	uint8_t core_id = apic::get_core_id_fast();
	apic::init_lapic();
	apic::send_ipi(0, INTERRUPT_ENTERED_MAIN);

	while (true)
	{
	}
}

namespace multicore
{

}
