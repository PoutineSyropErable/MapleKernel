#include "apic.hpp"
#include "assert.h"
#include "cast.h"
#include "cpuid.hpp"
#include "cpuid_results.hpp"

struct apic::apic_support apic::has_apic()
{

	struct cpuid::cpuid_verified_result cpuid_1 = cpuid::call_cpuid(cpuid::CpuidFunction::ProcessorInfo, {.raw = 0});
	if (cpuid_1.has_error())
	{
		abort_msg("If we can't even get cpuid1, we are fucked anyway\n");
	}

	struct cpuid_basic_edx edx = BITCAST(struct cpuid_basic_edx, cpuid_1.regs.edx);
	struct cpuid_basic_ecx ecx = BITCAST(struct cpuid_basic_ecx, cpuid_1.regs.ecx);
	return apic::apic_support{edx.apic, ecx.x2apic};
}

// This will actually be implemented in an assembly file
extern "C" void core_bootstrap()
{
}

// Idk if this should be implemented here
void apic::core_main()
{
}

uint8_t apic::get_core_id()
{
	// TODO
	return 0;
}

extern "C" uint8_t apic_get_core_id()
{
	return apic::get_core_id();
}

void apic::wake_core(uint8_t core_id, void core_bootstrap(), void core_main())
{
}
void apic::wait_till_interrupt(uint8_t interrupt_number)
{
}
