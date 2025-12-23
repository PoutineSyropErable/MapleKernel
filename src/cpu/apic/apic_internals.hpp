#pragma once
#include "apic.hpp"
#include "special_pointers.hpp"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

namespace apic
{

enum class lapic_registers_offset : uint16_t
{
	lapic_id	  = 0x20, // 0x20, Read/Write
	lapic_version = 0x30, // 0x30, Read only

	task_priority			  = 0x80, // 0x80, Read/Write
	arbitration_priority	  = 0x90, // 0x90, Read only
	process_priority		  = 0xA0, // 0xA0, Read only
	end_of_interrupt		  = 0xB0, // 0xB0, Write only
	remote_read				  = 0xC0, // 0xC0, Read Only
	logical_destination		  = 0xD0, // 0xD0, Read/Write
	destination_format		  = 0xE0, // 0xE0, Read/Write
	spurious_interrupt_vector = 0xF0, // 0xF0, Read/Write

	in_service_0 = 0x100, // 0x100, Read Only
	in_service_1 = 0x110, // 0x110, Read Only
	in_service_2 = 0x120, // 0x120, Read Only
	in_service_3 = 0x130, // 0x130, Read Only
	in_service_4 = 0x140, // 0x140, Read Only
	in_service_5 = 0x150, // 0x150, Read Only
	in_service_6 = 0x160, // 0x160, Read Only
	in_service_7 = 0x170, // 0x170, Read Only

	trigger_mode_0 = 0x180, // 0x180, Read only
	trigger_mode_1 = 0x190, // 0x190, Read only
	trigger_mode_2 = 0x1a0, // 0x1a0, Read only
	trigger_mode_3 = 0x1b0, // 0x1b0, Read only
	trigger_mode_4 = 0x1c0, // 0x1c0, Read only
	trigger_mode_5 = 0x1d0, // 0x1d0, Read only
	trigger_mode_6 = 0x1e0, // 0x1e0, Read only
	trigger_mode_7 = 0x1f0, // 0x1f0, Read only

	interrupt_request_0 = 0x200, // 0x200, Read only
	interrupt_request_1 = 0x210, // 0x210, Read only
	interrupt_request_2 = 0x220, // 0x220, Read only
	interrupt_request_3 = 0x230, // 0x230, Read only
	interrupt_request_4 = 0x240, // 0x240, Read only
	interrupt_request_5 = 0x250, // 0x250, Read only
	interrupt_request_6 = 0x260, // 0x260, Read only
	interrupt_request_7 = 0x270, // 0x270, Read only

	error_status = 0x280, // 0x280, Read Only
	lvt_cmci	 = 0x2f0, // 0x2f0, Read/Write, LVT Corrected Machine Check Interrupt (CMCI) Register

	command_low	 = 0x300, // 0x300, Read/Write
	command_high = 0x310, // 0x310, Read/Write

	lvt_timer							= 0x320, // 0x320, Read/Write
	lvt_thermal_sensor					= 0x330, // 0x330, Read/Write
	lvt_performance_monitoring_counters = 0x340, // 0x340, Read/Write
	lvt_lint0							= 0x350, // 0x350, Read/Write
	lvt_lint1							= 0x360, // 0x360, Read/Write
	lvt_error							= 0x370, // 0x370, Read/Write

	initial_count = 0x380, // 0x380, Read/Write
	current_count = 0x390, // 0x390, Read Only

	divide_configuration_register = 0x3E0, // 0x3E0, Read / Write

};

struct __attribute__((packed)) interrupt_command_register_low
{
	uint8_t	 vector_number;
	uint8_t	 delivery_mode : 3;
	bool	 destination_mode : 1;
	bool	 delivery_status : 1; // cleared when the interrupt has been accepted by the target. Always wait till cleared
	bool	 _reserved : 1;
	bool	 init_lvl_deassert_clear : 1;
	bool	 init_lvl_deassert_set : 1;
	uint8_t	 _reserved2 : 2;	   // bit 16-17
	uint8_t	 destination_type : 2; // bit 18-19
	uint16_t _reserved3 : 12;	   // bit 20-31
};
STATIC_ASSERT(sizeof(interrupt_command_register_low) == 4, "ICR low must be 32 bit");

struct __attribute__((packed)) interrupt_command_register_high
{
	uint32_t reserved : 24 = 0;
	uint32_t local_apic_id_of_target : 4;
	// Putting nothing else here makes the highest 4 bits reserved, and unaccessible

	// Default constructor
	// constexpr interrupt_command_register_high() = default;
};

STATIC_ASSERT(sizeof(interrupt_command_register_high) == 4, "ICR high must be 32 bit");

#define GET_ADDRESS(ToType, addr, offset)                                                                                                  \
	({                                                                                                                                     \
		uintptr_t		 _addr = (uintptr_t)(addr) + (uintptr_t)(offset);                                                                  \
		volatile ToType *ret   = (volatile ToType *)(_addr);                                                                               \
		ret;                                                                                                                               \
	})

extern volatile void *lapic_address;

template <typename ToType> constexpr volatile ToType *get_mmio_address(volatile void *base, lapic_registers_offset offset)
{
	return reinterpret_cast<volatile ToType *>(reinterpret_cast<uintptr_t>(base) + (uint16_t)offset);
}

constexpr enum lapic_registers_offset add_off(lapic_registers_offset lapic_offset, uint8_t offset)
{
	uint8_t loff = (uint8_t)lapic_offset + offset;
	return (lapic_registers_offset)loff;
}

class LapicRegisters
{
  public:
	std::set_once_ptr<volatile uint32_t>	   lapic_id;
	std::set_once_ptr<volatile uint32_t>	   lapic_version;
	std::set_once_ptr<volatile uint32_t>	   task_priority;
	std::set_once_ptr<volatile const uint32_t> arbitration_priority;

	std::set_once_ptr<volatile const uint32_t> in_service[8];
	std::set_once_ptr<volatile const uint32_t> trigger_mode[8];
	std::set_once_ptr<volatile const uint32_t> interrupt_request[8];

	std::set_once_ptr<volatile interrupt_command_register_low>	command_low;
	std::set_once_ptr<volatile interrupt_command_register_high> command_high;

	void init()
	{
		lapic_id.set(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::lapic_id));
		lapic_version.set(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::lapic_version));
		task_priority.set(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::task_priority));
		arbitration_priority.set(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::arbitration_priority));

		for (uint8_t i = 0; i < 8; i++)
		{
			in_service[i].set(get_mmio_address<uint32_t>(lapic_address, add_off(lapic_registers_offset::in_service_0, 0x10 * i)));
			trigger_mode[i].set(get_mmio_address<uint32_t>(lapic_address, add_off(lapic_registers_offset::trigger_mode_0, 0x10 * i)));
			interrupt_request[i].set(
				get_mmio_address<uint32_t>(lapic_address, add_off(lapic_registers_offset::interrupt_request_0, 0x10 * i)));
		}

		command_low.set(get_mmio_address<interrupt_command_register_low>(lapic_address, lapic_registers_offset::command_low));
		command_high.set(get_mmio_address<interrupt_command_register_high>(lapic_address, lapic_registers_offset::command_high));
	}

	void test()
	{
		// Write first to high, then to low
		uint32_t la							  = *lapic_id;
		command_high->local_apic_id_of_target = 5;
		interrupt_command_register_high ch{.local_apic_id_of_target = 5};
		command_low->vector_number = 1;
	}

  private:
};

extern LapicRegisters g_lapic_register;

} // namespace apic
