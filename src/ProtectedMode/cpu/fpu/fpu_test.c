#include "fpu.h"
#include "stdio.h"

void test_sse_alignment(void)
{
	kprintf("=== Testing SSE with Alignment ===\n");

	// Check CR4 (fix the print)
	uint32_t cr4;
	asm volatile("mov %%cr4, %0" : "=r"(cr4));
	kprintf("CR4 = %h:8\n", cr4);
	kprintf("  OSFXSR (bit 9) = %d\n", (cr4 >> 9) & 1);
	kprintf("  OSXMMEXCPT (bit 10) = %d\n", (cr4 >> 10) & 1);
	kprintf("  OSXSAVE (bit 18) = %d\n", (cr4 >> 18) & 1);

	// Check MXCSR
	uint32_t mxcsr;
	asm volatile("stmxcsr %0" : "=m"(mxcsr));
	kprintf("MXCSR = %h:8\n", mxcsr);

	// CRITICAL: Check current stack alignment
	uint32_t esp, ebp;
	asm volatile("mov %%esp, %0" : "=r"(esp));
	asm volatile("mov %%ebp, %0" : "=r"(ebp));

	kprintf("\n=== Stack Alignment Check ===\n");
	kprintf("ESP = %h:8 (align: %d bytes)\n", esp, esp & 0xF);
	kprintf("EBP = %h:8 (align: %d bytes)\n", ebp, ebp & 0xF);

	// Test 1: Simple SSE operation (should work)
	{
		float a = 1.5f, b = 2.5f, result;
		asm volatile("movss %1, %%xmm0\n"
					 "movss %2, %%xmm1\n"
					 "addss %%xmm1, %%xmm0\n"
					 "movss %%xmm0, %0"
			: "=m"(result)
			: "m"(a), "m"(b)
			: "xmm0", "xmm1");
		kprintf("Test 1 - Simple addss: %f + %f = %f\n", a, b, result);
	}

	// Test 2: movaps with aligned stack (should work)
	{
		// Create 16-byte aligned buffer on stack
		float buffer[4] __attribute__((aligned(16))) = {1.0f, 2.0f, 3.0f, 4.0f};
		float result[4] __attribute__((aligned(16)));

		kprintf("Test 2 - Buffer address: 0x%x (align: %d)\n", buffer, ((uint32_t)buffer) & 0xF);
		kprintf("Test 2 - Result address: 0x%x (align: %d)\n", result, ((uint32_t)result) & 0xF);

		asm volatile("movaps %1, %%xmm0\n" // Load aligned data
					 "movaps %%xmm0, %0\n" // Store aligned data
			: "=m"(result)
			: "m"(buffer)
			: "xmm0");

		kprintf("Test 2 - movaps SUCCESS: [%f, %f, %f, %f]\n", result[0], result[1], result[2], result[3]);
	}

	// Test 3: movaps with misaligned stack (should crash)
	{
		kprintf("\nTest 3 - Testing misaligned access (might crash!)\n");

		// Force misalignment by offsetting
		char   misalign_buffer[20];							// Extra bytes to allow shifting
		float *misaligned = (float *)(misalign_buffer + 1); // Guaranteed misaligned!

		float temp_result[4] __attribute__((aligned(16)));

		kprintf("Test 3 - Misaligned address: 0x%x (align: %d)\n", misaligned, ((uint32_t)misaligned) & 0xF);

		// Initialize with some values
		for (int i = 0; i < 4; i++)
		{
			misaligned[i] = (float)(i + 1);
		}

		// Try movaps - WILL CRASH if alignment is enforced!
		asm volatile("movups %1, %%xmm0\n" // Use movups (unaligned) instead
					 "movaps %%xmm0, %0\n" // Store to aligned destination
			: "=m"(temp_result)
			: "m"(misaligned[0])
			: "xmm0");

		kprintf("Test 3 - movups succeeded (using unaligned load)\n");
	}

	// Test 4: Exact crash reproduction
	{
		kprintf("\nTest 4 - Reproducing exact crash pattern\n");

		// Create stack frame similar to the crashing function
		uint32_t ebp_before, esp_before;
		asm volatile("mov %%ebp, %0" : "=r"(ebp_before));
		asm volatile("mov %%esp, %0" : "=r"(esp_before));

		kprintf("Test 4 - EBP = 0x%x (align: %d)\n", ebp_before, ebp_before & 0xF);
		kprintf("Test 4 - ESP = 0x%x (align: %d)\n", esp_before, esp_before & 0xF);

		// Calculate the crashing address
		uint32_t crash_addr = ebp_before - 0x1b8;
		kprintf("Test 4 - [EBP-0x1B8] = 0x%x (align: %d) ", crash_addr, crash_addr & 0xF);

		if ((crash_addr & 0xF) == 0)
		{
			kprintf("- PROPERLY ALIGNED\n");
		}
		else
		{
			kprintf("- MISALIGNED! (This is the problem)\n");
		}

		// Try to simulate what the compiler is doing
		struct test_struct
		{
			uint32_t data[4];
		} __attribute__((aligned(16)));

		struct test_struct aligned_struct;
		uint32_t		  *test_ptr = (uint32_t *)crash_addr;

		kprintf("Test 4 - Would access memory at 0x%x\n", test_ptr);
	}

	// Test 5: What the compiler-generated code is doing
	{
		kprintf("\nTest 5 - Simulating compiler optimization\n");

		// Simulating: movd 0x214434, %xmm0
		uint32_t constant_data = 0x214434;
		float	 xmm_test[4] __attribute__((aligned(16)));

		asm volatile("movd %1, %%xmm0\n"
					 "movd %%xmm0, %0\n"
			: "=m"(xmm_test[0])
			: "r"(constant_data)
			: "xmm0");

		kprintf("Test 5 - movd constant -> xmm0 worked\n");
	}

	kprintf("\n=== SSE Test Complete ===\n");
}
