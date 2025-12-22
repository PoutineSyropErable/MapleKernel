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
