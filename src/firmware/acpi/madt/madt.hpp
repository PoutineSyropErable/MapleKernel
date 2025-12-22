#pragma once
#include "acpi.h"

namespace acpi
{

namespace madt
{

// ========== Start of madt
struct MADT
{
};

enum class EntryType
{
	processor_local_apic				  = 0,
	io_apic								  = 1,
	io_apic_interrupt_source_override	  = 2,
	io_apic_non_maskable_interrupt_source = 3,
	local_apic_non_maskable_interrupt	  = 4,
	local_apic_address_override			  = 5,
	processor_local_x2apic				  = 6,
};

// ========== End of madt
} // namespace madt
} // namespace acpi
