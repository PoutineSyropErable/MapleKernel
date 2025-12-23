#include "apic.hpp"
#include "apic_internals.hpp"
#include "assert.h"
#include "cast.h"
#include "cpuid.hpp"
#include "cpuid_results.hpp"
#include "intrinsics.h"

volatile void *apic::lapic_address;
volatile void *apic::io_appic_address;

apic::LapicRegisters apic::g_lapic_register;

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

enum apic::error apic::init_apic()
{
	g_lapic_register.init();
	// There's no actual hardware apic init. Only lapic (core local, must do for every core)
	return apic::error::none;
}

extern "C" uint8_t test_cmd()
{

	const volatile apic::interrupt_command_register_high *reg = (const volatile apic::interrupt_command_register_high *)0x50;
	const apic::interrupt_command_register_high			  res = *reg;

	return res.local_apic_id_of_target;
}

enum apic::error apic::init_lapic()
{

	return apic::error::none;
}

void apic::calibrate_lapic_timer()
{
}

enum apic::error apic::init_io_apic()
{

	return apic::error::none;
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

/* =========================== C OSDEV ===================== */
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

/** returns a 'true' value if the CPU supports APIC
 *  and if the local APIC hasn't been disabled in MSRs
 *  note that this requires CPUID to be supported.
 */

/* Set the physical address for local APIC registers */
void cpu_set_apic_base(uintptr_t apic)
{
	uint32_t edx = 0;
	uint32_t eax = (apic & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;

#ifdef __PHYSICAL_MEMORY_EXTENSION__
	edx = (apic >> 32) & 0x0f;
#endif

	write_msr(IA32_APIC_BASE_MSR, eax, edx);
}

/**
 * Get the physical address of the APIC registers page
 * make sure you map it to virtual memory ;)
 */
uintptr_t cpu_get_apic_base()
{
	uint32_t eax, edx;
	read_msr(IA32_APIC_BASE_MSR, &eax, &edx);

#ifdef __PHYSICAL_MEMORY_EXTENSION__
	return (eax & 0xfffff000) | ((edx & 0x0f) << 32);
#else
	return (eax & 0xfffff000);
#endif
}

void enable_apic()
{
	/* Section 11.4.1 of 3rd volume of Intel SDM recommends mapping the base address page as strong uncacheable for correct APIC operation.
	 */

	/* Hardware enable the Local APIC if it wasn't enabled */
	cpu_set_apic_base(cpu_get_apic_base());

	/* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
	// write_reg(0xF0, ReadRegister(0xF0) | 0x100);
}
