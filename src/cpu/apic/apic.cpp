#include "apic.hpp"
#include "apic_internals.hpp"
#include "assert.h"
#include "cast.h"
#include "cpuid.hpp"
#include "cpuid_results.hpp"
#include "intrinsics.h"
#include "multicore_gdt.hpp"
// This file needs -fno-strict-aliasing
#include "multicore.h"
#include "pit.hpp"
#include "pit_internals.h"

using namespace apic;

volatile void *apic::lapic_address;
volatile void *apic::io_appic_address;

volatile uint8_t last_interrupt_received[MAX_CORE_COUNT][MAX_CORE_COUNT];
volatile bool	 core_has_booted[MAX_CORE_COUNT];			 // [i = reciever][ j = sender]
volatile bool	 master_tells_core_to_start[MAX_CORE_COUNT]; // [i = reciever][ j = sender]
void (*core_mains[8])();

// Private (To this file) and global
static LapicRegisters gp_lapic_register;

// __attribute__((fastcall)) void apic::LapicRegisters::send_command(interrupt_command_low_register low, interrupt_command_high_register
// high)
// {
// 	// Do something
// 	command_high.write(high);
// 	command_low.write(low);
// }

extern "C" uint8_t test_cmd()
{

	gp_lapic_register.send_command(
		{
			.init_lvl_deassert_clear = 1,
		},
		{.local_apic_id_of_target = 1});

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

uint8_t apic::get_core_id_fast()
{
	// requires it to have been set once by using the slower method
	return multicore_gdt::get_fs_struct()->core_id;
}

extern "C" uint8_t apic_get_core_id()
{
	return apic::get_core_id_fast();
}

void send_init_ipi(uint8_t core_id)
{
}

void send_sipi(uint8_t core_id, void core_bootstrap())
{
}

enum error apic::wake_core(uint8_t core_id, void (*core_bootstrap)(), void (*core_main)())
{

	// Sets up the main loop that core will use.
	core_mains[core_id] = core_main;

	send_init_ipi(core_id);
	pit::wait(10.f / 1000.f);
	// send_init_ipi(core_id, deassert);
	pit::wait(10.f / 1000.f);

	// Send Sipi
	send_sipi(core_id, core_bootstrap);
	volatile uint32_t finished = false;
	int				  err	   = pit::short_timeout(0.001f, &finished, true);
	if (err)
	{
		kprintf("There was an error waiting for the short timeout\n");
		return error::bad_time;
	}
	while (true)
	{

		if (core_has_booted[core_id])
		{

			master_tells_core_to_start[core_id] = true;
			return apic::error::none;
		}
		else if (finished)
		{
			goto long_poll;
		}
	}

long_poll:
	send_sipi(core_id, core_bootstrap);
	constexpr uint16_t long_timeout_ms = 1000;
	constexpr uint16_t short_timeut_ms = 50;
	constexpr uint8_t  timeout_count   = long_timeout_ms / short_timeut_ms;
	for (uint8_t i = 0; i < timeout_count; i++)
	{
		pit::short_timeout((float)short_timeut_ms / 1000.f, &finished, false);
		while (true)
		{
			if (core_has_booted[core_id])
			{

				master_tells_core_to_start[core_id] = true;
				return apic::error::none;
			}
			else if (finished)
			{
				break;
			}
		}
	}
	kprintf("Boot timeout happened\n");
	return apic::error::boot_core_timeout;
}

void apic::wait_till_interrupt(uint8_t interrupt_vector)
{
	assert(interrupt_vector != NO_INTERRUPT, "make no sense to wait for a never interrupt\n");
	uint32_t core_id = get_core_id_fast();
	while (true)
	{
		__hlt();
		// Check if any of the sender sent it.
		for (uint8_t sender_id = 0; sender_id < runtime_core_count; sender_id++)
		{
			uint8_t lir = last_interrupt_received[core_id][sender_id];
			if (lir == interrupt_vector)
			{
				last_interrupt_received[core_id][sender_id] = NO_INTERRUPT;
				break;
				// TODO: Warning, some multicore shenanigans and race condition possible here
				// Should be safe when used to init core and wait till interrupt
			}
		}
	}
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
