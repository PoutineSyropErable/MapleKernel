#include "apic.hpp"
#include "apic_internals.hpp"
#include "apic_io.hpp"
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

#define TIMEOUT_IPI_PENDING 1000'000

using namespace apic;

volatile uint8_t last_interrupt_received[MAX_CORE_COUNT][MAX_CORE_COUNT];
volatile bool	 core_has_booted[MAX_CORE_COUNT];			 // [i = reciever][ j = sender]
volatile bool	 master_tells_core_to_start[MAX_CORE_COUNT]; // [i = reciever][ j = sender]
void (*volatile core_mains[MAX_CORE_COUNT])();

// Private (To this file) and global
static LapicRegisters lapic;

// __attribute__((fastcall)) void apic::LapicRegisters::send_command(interrupt_command_low_register low, interrupt_command_high_register
// high)
// {
// 	// Do something
// 	command_high.write(high);
// 	command_low.write(low);
// }

extern "C" uint8_t test_cmd()
{
	lapic.send_command(
		{
			.vector_number	  = 0,
			.delivery_mode	  = delivery_mode::init,
			.destination_mode = destination_mode::physical,
			.destination_type = destination_type::normal,
		},
		{.local_apic_id_of_target = 1});

	return 1;
}

bool					  apic::support_tsc_deadline = false;
struct apic::apic_support apic::has_apic()
{

	struct cpuid::cpuid_verified_result cpuid_1 = cpuid::call_cpuid(cpuid::CpuidFunction::ProcessorInfo, {.raw = 0});
	if (cpuid_1.has_error())
	{
		abort_msg("If we can't even get cpuid1, we are fucked anyway\n");
	}

	struct cpuid_basic_edx edx = BITCAST(struct cpuid_basic_edx, cpuid_1.regs.edx);
	struct cpuid_basic_ecx ecx = BITCAST(struct cpuid_basic_ecx, cpuid_1.regs.ecx);
	apic::support_tsc_deadline = ecx.tsc_deadline;
	return apic::apic_support{edx.apic, ecx.x2apic, ecx.tsc_deadline};
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

	apic::spurious_interrupt_vector_register spivr = lapic.spurious_interrupt_vector.read();
	spivr.vector = apic::spurious_interrupt_vector; // Write interrupt handler 56 to handle spurious interrupts
	// Spurious means fake, it happens when an interrupt is recieved with lower priority then current
	// aka, an ignored interrupt. Then the spurious is called
	spivr.apic_enable  = true;
	spivr.eoi_suppress = false;
	lapic.spurious_interrupt_vector.write(spivr);

	uint8_t core_id = get_core_id_fast();
	if (core_id != 0)
	{
		lapic.lvt_lint0.write({.vector_number = 0, .mask = mask::disable});
	}
	// lapic.lvt_lint1.write({.vector_number = 0, .mask = mask::enable});
	// lint1 is for nmi, and i don't have any of them for now so, All is good

	lapic.task_priority.write({
		.task_priority_subclass = task_priority_subclass::default_, .task_priority = task_priority::all, .res = 0,
		// Just write 0 there
	});
	return apic::error::none;
}

void apic::calibrate_lapic_timer()
{
}

uint8_t apic::get_core_id()
{
	return lapic.lapic_id.read().apic_id;
}

void apic::send_eoi()
{
	// forced to use this
	lapic.send_eoi();
}

extern "C" void apic_send_eoi()
{
	// Sadly, can't inline this call
	lapic.send_eoi();

	// mov dword [ EOI_MMIO_ADDR ], 0
	// mov    DWORD PTR ds:0xfee000b0,0x0
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
	lapic.send_command(
		{
			.vector_number	  = 0,
			.delivery_mode	  = delivery_mode::init,
			.destination_mode = destination_mode::physical,
			.destination_type = destination_type::normal,
		},
		{.local_apic_id_of_target = core_id});

	for (uint32_t i = 0; i < TIMEOUT_IPI_PENDING; i++)
	{
		bool recieved_pending = lapic.command_low.read().delivery_status_pending_ro;
		if (!recieved_pending)
		{
			goto wait;
		}
	}
	return error::timeout_sending_ipi;

wait:
	pit::wait(10.f / 1000.f);

	lapic.send_command(
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
		bool recieved_pending = lapic.command_low.read().delivery_status_pending_ro;
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

	lapic.send_command(
		{
			.vector_number	  = static_cast<uint8_t>(cb / 0x1000),
			.delivery_mode	  = delivery_mode::start_up,
			.destination_mode = destination_mode::physical,
			.destination_type = destination_type::normal,
		},
		{.local_apic_id_of_target = core_id});

	for (uint32_t i = 0; i < TIMEOUT_IPI_PENDING; i++)
	{
		bool recieved_pending = lapic.command_low.read().delivery_status_pending_ro;
		if (!recieved_pending)
		{
			return error::none;
		}
	}
	return error::timeout_sending_ipi;
}

reentrant_lock_t last_interrupt_received_lock{.state = 0};
enum error		 apic::send_ipi(uint8_t core_id, uint8_t int_vector)
{

#ifdef DEBUG
	assert(core_id <= 0b1111, "Won't fit\n");
#endif

	uint8_t this_core_id = get_core_id_fast();
	reentrant_lock(&last_interrupt_received_lock);
	last_interrupt_received[core_id][this_core_id] = int_vector;
	reentrant_unlock(&last_interrupt_received_lock);
	// kprintf("Core %u sending interrupt %u to core %u\n\n", this_core_id, int_vector, core_id);
	// without this print, it fucks
	// TODO: Fix the weird deadlocks

	lapic.send_command(
		{
			.vector_number = int_vector,
			// The rest default
		},
		{.local_apic_id_of_target = core_id});

	for (uint32_t i = 0; i < TIMEOUT_IPI_PENDING; i++)
	{
		bool recieved_pending = lapic.command_low.read().delivery_status_pending_ro;
		if (!recieved_pending)
		{
			kprintf("Quit at i = %u\n", i);
			return error::none;
		}
	}

	kprintf("Timed out waiting for sending ipi\n");
	return error::timeout_sending_ipi;
}

enum error apic::wake_core(uint8_t core_id, void (*core_bootstrap)(), void (*core_main)())
{

	kprintf("Trying to wake %u\n", core_id);
	error apic_err = send_init(core_id);
	if ((uint8_t)apic_err)
	{
		return apic_err;
	}

	// Sets up the main loop that core will use.
	kprintf("Core main = %h\n", core_main);
	core_mains[core_id] = core_main;

	// Send Sipi
	pit::wait(10.f / 1000.f);
	apic_err = send_sipi(core_id, core_bootstrap);
	if ((uint8_t)apic_err)
	{
		return apic_err;
	}

	volatile uint32_t finished = false;
	int				  pit_err  = 0;
	pit_err					   = pit::short_timeout_async(0.001f, &finished, true);
	if (pit_err)
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
		pit_err = pit::short_timeout_async((float)short_timeut_ms / 1000.f, &finished, false);
		if (pit_err)
		{
			kprintf("There was an error waiting for the short timeout (In Long Poll), i = %u\n", i);
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
		// If it's sent while its trying to take the lock
		for (uint8_t sender_id = 0; sender_id < runtime_core_count; sender_id++)
		{
			uint8_t lir = last_interrupt_received[core_id][sender_id];
			// kprintf("Lir = %u\n", lir);
			if (lir == interrupt_vector)
			{
				reentrant_lock(&last_interrupt_received_lock);
				last_interrupt_received[core_id][sender_id] = NO_INTERRUPT;
				reentrant_unlock(&last_interrupt_received_lock);
				irq_restore(flags);
				return;
				// TODO: Warning, some multicore shenanigans and race condition possible here
				// Should be safe when used to init core and wait till interrupt

				// Check time, set time race condition.
				// This is a nightmare, to work on more later

				// Also, what if it sends an interrupt halts after the interrupt is recieved.
				// Then it infinites loop and never wake up
			}
		}
	}
}
