#pragma once
#include <stddef.h>
#include <stdint.h>

namespace apic
{

struct apic_support
{
	bool apic : 1;
	bool x2apic : 1;
};

struct apic_support has_apic();

} // namespace apic
