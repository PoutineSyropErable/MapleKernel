#pragma once
#include "apic_internals.hpp"

namespace apic
{
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

} // namespace apic
