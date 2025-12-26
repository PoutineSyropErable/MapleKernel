#include "apic.hpp"
#include "apic_internals.hpp"
#include "assert.h"
#include "cast.h"
#include "cpuid.hpp"
#include "cpuid_results.hpp"
#include "intrinsics.h"
#include "multicore_gdt.hpp"
// This file needs -fno-strict-aliasing

using namespace apic;

volatile void *apic::lapic_address;
volatile void *apic::io_appic_address;

// Private (To this file) and global
static LapicRegisters gp_lapic_register;

void apic::LapicRegisters::doX(uint8_t x, float y)
{
	// Do something
}

extern "C" uint8_t test_cmd()
{

	const std::mmio_ptr<apic::interrupt_command_register_high> ptr((volatile apic::interrupt_command_register_high *)0xb00000);
	apic::interrupt_command_register_high					   local{.local_apic_id_of_target = 5};
	ptr.write(local);

	return 1;
	// return res.local_apic_id_of_target;
}

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
	// Technically, I'm only implementing it for the apic (x1).
	// The mmio version, not the msr version.
}

void apic::read_apic_msr(apic_msr_eax *eax, apic_msr_edx *edx)
{
	uint32_t eax_raw, edx_raw;
	read_msr(apic::apic_msr, &eax_raw, &edx_raw);

	*eax = from_uint32<apic_msr_eax>(eax_raw);
	*edx = from_uint32<apic_msr_edx>(edx_raw);
}
void apic::write_apic_msr(apic::apic_msr_eax eax, apic::apic_msr_edx edx)
{

	uint32_t eax_raw = to_uint32<apic_msr_eax>(eax);
	uint32_t edx_raw = to_uint32<apic_msr_edx>(edx);
	write_msr(apic::apic_msr, eax_raw, edx_raw);
}

// This assume 4GB max,
// edx is for the rest of the address;
uint32_t apic::get_base_address()
{
	apic_msr_eax eax;
	apic_msr_edx edx;
	read_apic_msr(&eax, &edx);
	assert(!eax.x2apic, "x2 apic doesn't work with base address mmio, but msr!\n");
	return eax.base_low_page_idx << 12;
}

// This assume 4GB max,
// edx is for the rest of the address;
void apic::set_base_address(uint32_t base_address)
{
	constexpr uint32_t four_kb	 = (1 << 12);
	constexpr uint32_t page_mask = ((four_kb << 1) - 1);
	assert((base_address & page_mask) == 0, "Base address must be page aligned!\n");

	apic_msr_eax eax;
	apic_msr_edx edx;
	if (false)
	{
		edx.base_high = 0b1111; // max base_high value
	}
	read_apic_msr(&eax, &edx);
	assert(!eax.x2apic, "x2 apic doesn't work with base address mmio, but msr!\n");
	eax.base_low_page_idx = (base_address >> 12);
	write_apic_msr(eax, edx);
}

enum apic::error apic::init_apic()
{
	// Done once for global enable.
	gp_lapic_register.init();

	apic_msr_eax eax;
	apic_msr_edx edx;
	read_apic_msr(&eax, &edx);
	eax.apic_enable = true;
	assert(eax.is_bootstrap_processor, "This must only be executed once by the bootstrap processor\n");
	write_apic_msr(eax, edx);

	return apic::error::none;
}

uint32_t get_core_id_fast()
{
	return multicore_gdt::get_fs_struct()->core_id;
}

enum apic::error apic::init_lapic()
{

	apic::spurious_interrupt_vector_register spivr					= gp_lapic_register.spurious_interrupt_vector.read();
	constexpr uint8_t						 number_of_reserved_int = 32;
	constexpr uint8_t						 number_of_apic_io_irq	= 24;
	constexpr uint8_t						 vector_when_error		= (number_of_reserved_int + number_of_apic_io_irq);
	spivr.vector	   = vector_when_error; // Write interrupt handler 56 to handle spurious interrupts
	spivr.apic_enable  = true;
	spivr.eoi_suppress = false;
	gp_lapic_register.spurious_interrupt_vector.write(spivr);
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
	return gp_lapic_register.lapic_id.read();
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
