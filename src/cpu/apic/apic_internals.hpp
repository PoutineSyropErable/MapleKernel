#pragma once
#include "apic.hpp"
#include "special_pointers.hpp"
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

};

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

class LapicRegistersV1
{
  public:
	// *const: Const pointer.
	// const uint32_t : const value
	volatile uint32_t *const	   lapic_id;
	volatile uint32_t *const	   lapic_version;
	volatile uint32_t *const	   task_priority;
	volatile const uint32_t *const arbitration_priority;

	LapicRegistersV1()
		: lapic_id(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::lapic_id)),
		  lapic_version(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::lapic_version)),
		  task_priority(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::task_priority)),
		  arbitration_priority(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::arbitration_priority))
	{
	}

  private:
};

class LapicRegistersV2
{
  public:
	std::set_once_ptr<volatile uint32_t>	   lapic_id;
	std::set_once_ptr<volatile uint32_t>	   lapic_version;
	std::set_once_ptr<volatile uint32_t>	   task_priority;
	std::set_once_ptr<volatile const uint32_t> arbitration_priority;

	void init()
	{
		lapic_id.set(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::lapic_id));
		lapic_version.set(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::lapic_version));
		task_priority.set(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::task_priority));
		arbitration_priority.set(get_mmio_address<uint32_t>(lapic_address, lapic_registers_offset::arbitration_priority));
	}

  private:
};

extern LapicRegistersV2 g_lapic_register;

} // namespace apic
