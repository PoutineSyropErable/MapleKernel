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
	// kprintf("Testing special pointers\n\n");

	// volatile Reg16x2 reg1{.low = 5, .high = 6};

	// Create proxies
	const std::mmio_pointer<Reg16x2> ptr((volatile Reg16x2 *)0xb00000);

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
	Reg16x2 r = ptr.read();
}
