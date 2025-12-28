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

#define TIMEOUT_IPI_PENDING 10'000

using namespace apic;

volatile uint8_t last_interrupt_received[MAX_CORE_COUNT][MAX_CORE_COUNT];
volatile bool	 core_has_booted[MAX_CORE_COUNT];			 // [i = reciever][ j = sender]
volatile bool	 master_tells_core_to_start[MAX_CORE_COUNT]; // [i = reciever][ j = sender]
void (*volatile core_mains[8])();

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
			.vector_number	  = 0,
			.delivery_mode	  = delivery_mode::init,
			.destination_mode = destination_mode::physical,
			.destination_type = destination_type::normal,
		},
		{.local_apic_id_of_target = 1});

	return 1;
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
	spivr.vector = vector_when_error; // Write interrupt handler 56 to handle spurious interrupts
	// Spurious means fake, it happens when an interrupt is recieved with lower priority then current
	// aka, an ignored interrupt. Then the spurious is called
	spivr.apic_enable  = true;
	spivr.eoi_suppress = false;
	gp_lapic_register.spurious_interrupt_vector.write(spivr);
	return apic::error::none;
}

void apic::calibrate_lapic_timer()
{
}

uint8_t apic::get_core_id()
{
	return gp_lapic_register.lapic_id.read().apic_id;
}

uint8_t apic::get_core_id_fast()
{
	// requires it to have been set once by using the slower method
	uint32_t apic_id;
	__asm__ volatile("mov %%fs:0, %0" : "=r"(apic_id));
	return (uint8_t)apic_id; // Cast to uint8_t
}

extern "C" uint8_t apic_get_core_id()
{
	uint32_t apic_id;
	__asm__ volatile("mov %%fs:0, %0" : "=r"(apic_id));
	return (uint8_t)apic_id; // Cast to uint8_t
}

error send_init(uint8_t core_id)
{
	assert(core_id <= 0b1111, "Won't fit\n");
	gp_lapic_register.send_command(
		{
			.vector_number	  = 0,
			.delivery_mode	  = delivery_mode::init,
			.destination_mode = destination_mode::physical,
			.destination_type = destination_type::normal,
		},
		{.local_apic_id_of_target = core_id});

	for (uint32_t i = 0; i < TIMEOUT_IPI_PENDING; i++)
	{
		bool recieved_pending = gp_lapic_register.command_low.read().delivery_status_pending_ro;
		if (!recieved_pending)
		{
			goto wait;
		}
	}
	return error::timeout_sending_ipi;

wait:
	// pit::wait(10.f / 1000.f);

	gp_lapic_register.send_command(
		{
			.vector_number	  = 0,
			.delivery_mode	  = delivery_mode::init,
			.destination_mode = destination_mode::physical,
			.level			  = level::deassert,
			.trigger_mode	  = trigger_mode::level,
			.destination_type = destination_type::normal,
		},
		{.local_apic_id_of_target = core_id});

	for (uint32_t i = 0; i < TIMEOUT_IPI_PENDING; i++)
	{
		bool recieved_pending = gp_lapic_register.command_low.read().delivery_status_pending_ro;
		if (!recieved_pending)
		{
			return error::none;
		}
	}
	return error::timeout_sending_ipi;
}

error send_sipi(uint8_t core_id, void (*core_bootstrap)())
{

	uintptr_t cb = (uintptr_t)core_bootstrap;

	kprintf("Core bootstrap location: %h\n", core_bootstrap);
	assert(core_id <= 0b1111, "Won't fit\n");
	assert(cb < 0xFF * 0x1000, "Start address must be a 16bit address");
	assert((cb & 0xFFF) == 0, "SIPI start address must be 4 KB aligned");

	gp_lapic_register.send_command(
		{
			.vector_number	  = static_cast<uint8_t>(cb / 0x1000),
			.delivery_mode	  = delivery_mode::start_up,
			.destination_mode = destination_mode::physical,
			.destination_type = destination_type::normal,
		},
		{.local_apic_id_of_target = core_id});

	for (uint32_t i = 0; i < TIMEOUT_IPI_PENDING; i++)
	{
		bool recieved_pending = gp_lapic_register.command_low.read().delivery_status_pending_ro;
		if (!recieved_pending)
		{
			return error::none;
		}
	}
	return error::timeout_sending_ipi;
}

enum error apic::send_ipi(uint8_t core_id, uint8_t int_vector)
{

#ifdef DEBUG
	assert(core_id <= 0b1111, "Won't fit\n");
#endif

	uint8_t this_core_id						   = get_core_id_fast();
	last_interrupt_received[core_id][this_core_id] = int_vector;
	// kprintf("Core %u sending interrupt %u to core %u\n\n", this_core_id, int_vector, core_id);
	// without this print, it fucks
	// TODO: Fix the weird deadlocks

	gp_lapic_register.send_command(
		{
			.vector_number = int_vector,
			// The rest default
		},
		{.local_apic_id_of_target = core_id});

	for (uint32_t i = 0; i < TIMEOUT_IPI_PENDING; i++)
	{
		bool recieved_pending = gp_lapic_register.command_low.read().delivery_status_pending_ro;
		if (!recieved_pending)
		{
			return error::none;
		}
	}

	return error::timeout_sending_ipi;
}

enum error apic::wake_core(uint8_t core_id, void (*core_bootstrap)(), void (*core_main)())
{

	error apic_err = send_init(core_id);
	if ((uint8_t)apic_err)
	{
		return apic_err;
	}

	// Sets up the main loop that core will use.
	core_mains[core_id] = core_main;
	kprintf("Core main = %h\n", core_main);

	// Send Sipi
	// pit::wait(10.f / 1000.f);
	apic_err = send_sipi(core_id, core_bootstrap);
	if ((uint8_t)apic_err)
	{
		return apic_err;
	}

	volatile uint32_t finished = false;
	// int				  pit_err  = pit::short_timeout(0.001f, &finished, true);
	// if (pit_err)
	// {
	// 	kprintf("There was an error waiting for the short timeout\n");
	// 	return error::bad_time;
	// }
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
	apic_err = send_sipi(core_id, core_bootstrap);
	if ((uint8_t)apic_err)
	{
		return apic_err;
	}
	constexpr uint16_t long_timeout_ms = 1000;
	constexpr uint16_t short_timeut_ms = 50;
	constexpr uint8_t  timeout_count   = long_timeout_ms / short_timeut_ms;
	for (uint8_t i = 0; i < timeout_count; i++)
	{
		// pit::short_timeout((float)short_timeut_ms / 1000.f, &finished, false);
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
	uint32_t flags	 = irq_save();
	__sti();
	while (true)
	{
		__hlt();
		// Check if any of the sender sent it.
		for (uint8_t sender_id = 0; sender_id < runtime_core_count; sender_id++)
		{
			uint8_t lir = last_interrupt_received[core_id][sender_id];
			// kprintf("Lir = %u\n", lir);
			if (lir == interrupt_vector)
			{
				kprintf("recieved\n");
				last_interrupt_received[core_id][sender_id] = NO_INTERRUPT;
				irq_restore(flags);
				return;
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
