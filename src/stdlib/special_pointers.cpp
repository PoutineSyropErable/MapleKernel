#include "special_pointers.hpp"
#include "stdio.h"
#include <stdint.h>

struct __attribute__((packed)) Reg16x2
{
	uint32_t low : 16;
	uint32_t high : 16;
};

void test_special_pointers()
{
	kprintf("Testing special pointers\n\n");

	// volatile Reg16x2 reg1{.low = 5, .high = 6};

	// Create proxies
	const std::mmio_ptr<Reg16x2> ptr((volatile Reg16x2 *)0xb00000);

	// Local copies
	// Test writes deref
	// kprintf("\nTesting deref write (*proxy = local)\n");
	Reg16x2 val{.low = 7, .high = 8};
	ptr.write(val);

	// Test writes deref rvalue
	// kprintf("\nTesting deref write (*proxy = new)\n");
	ptr.write(Reg16x2{.low = 5, .high = 9});

	// Test reads deref
	// kprintf("\nTesting local read deref (local = *proxy)\n");
	// Reg16x2 r = ptr.read();
}

void __attribute__((fastcall)) test_special_pointers2(uintptr_t addr)
{

	uint8_t which = 0;

	if (which == 0)
	{
		volatile Reg16x2				  *ptr_raw = (volatile Reg16x2 *)0xb00000;
		const std::const_mmio_ptr<Reg16x2> ptr(ptr_raw);

		Reg16x2 val{.low = 7, .high = 8};
		ptr.write(val);
	}

	if (which == 1)
	{
		volatile Reg16x2				  *ptr_raw = (volatile Reg16x2 *)addr;
		const std::const_mmio_ptr<Reg16x2> ptr(ptr_raw);

		Reg16x2 val{.low = 7, .high = 8};
		ptr.write(val);
	}

	if (which == 10)
	{
		volatile Reg16x2			*ptr_raw = (volatile Reg16x2 *)addr;
		const std::mmio_ptr<Reg16x2> ptr(ptr_raw);

		Reg16x2 val{.low = 7, .high = 8};
		ptr.write(val);
	}

	// // Set-once read/write

	if (which == 2)
	{
		volatile Reg16x2			*ptr_raw = (volatile Reg16x2 *)addr;
		const std::mmio_ptr<Reg16x2> ptr(ptr_raw);

		std::set_once<std::mmio_ptr<Reg16x2>> global_mmio;
		global_mmio.set(ptr); // can only call once

		global_mmio.write(Reg16x2{.low = 5, .high = 2});
	}

	if (which == 3)
	{
		volatile Reg16x2			*ptr_raw = (volatile Reg16x2 *)addr;
		const std::mmio_ptr<Reg16x2> ptr(ptr_raw);

		std::set_once_ro<std::mmio_ptr<Reg16x2>> global_mmio_ro;

		global_mmio_ro.set(ptr); // can only call once
		Reg16x2 reg_read = global_mmio_ro.read();
		// Sadly read must use a stack variable to save the result.
		// It's so dumb. But, it's an impossible to remove optimisation
	}

	if (which == 4)
	{
		volatile Reg16x2			*ptr_raw = (volatile Reg16x2 *)addr;
		const std::mmio_ptr<Reg16x2> ptr(ptr_raw);

		std::set_once<std::mmio_ptr<Reg16x2>> global_mmio_wo;

		global_mmio_wo.set(ptr); // can only call once
		global_mmio_wo.write(Reg16x2{.low = 5, .high = 2});
	}

	if (which == 5)
	{
		volatile Reg16x2			*ptr_raw = (volatile Reg16x2 *)addr;
		const std::mmio_ptr<Reg16x2> ptr(ptr_raw);

		std::set_once_wo<std::mmio_ptr<Reg16x2>> global_mmio_wo;

		global_mmio_wo.set(ptr); // can only call once
								 // global_mmio_wo.write_m(Reg16x2{.low = 5, .high = 2});
	}
}
