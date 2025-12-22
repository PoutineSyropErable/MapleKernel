#include "acpi.hpp"
#include "apic.hpp"
#include "madt.h"
#include "madt.hpp"
#include "stdio.h"

void acpi::madt::bruh()
{
	kprintf("Bruh\n");
	bool has_pic = apic::has_apic();
	kprintf("Has apic: %b\n", has_pic);
}
