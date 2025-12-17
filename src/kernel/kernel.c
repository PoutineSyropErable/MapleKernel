#include "kernel.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "call_real16_wrapper.h"
#include "idt_master.h"
#include "kernel_cpp.h"
#include "kernel_zig.h"
#include "os_registers.c"
#include "pic.h"
#include "realmode_functions.h"

#include "stdio.h"
#include "stdlib.h"
#include "vga_terminal.h"

#include "ps2.h"
#include "ps2_controller.h"
#include "ps2_keyboard_public.h"
#include "ps2_mouse.h"
#include "ps2_mouse_public.h"

#include "framebuffer.h"
#include "framebuffer_multiboot.h"
#include "multiboot.h"

#include "kernel_helper.h"

#include "Main.h"
#include "cpuid.h"
#include "fpu.h"
#include "pit.h"

GDT_ROOT *GDT16_ROOT = &GDT16_DESCRIPTOR;

float fpu_add(float a, float b)
{
	float result;
	asm volatile("fld %1;"	// Load 'a' onto the FPU stack
				 "fadd %2;" // Add 'b' to ST(0)
				 "fstp %0;" // Store the result into 'result' and pop the stack
		: "=m"(result)		// output
		: "m"(a), "m"(b)	// inputs
	);
	return result;
}

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

void kernel_main(uint32_t mb2_info_addr, uint32_t magic, uint32_t is_proper_multiboot_32)
{

#if defined(QEMU)
	kprintf("\n\n=========Start of Kernel Main==========\n\n");
#endif

	uint32_t cpuid_supp_test = cpuid_supported_check();
	bool	 cpuid_supported = (cpuid_supp_test != 0);
	terminal_write_hex("Cpuid Support Test (must not be 0) =\n", cpuid_supp_test);
	if (!cpuid_supported)
	{
		terminal_writestring("We are fucked, cpuid isn't supported. I'm not gonna support dinosaur computers\n");
		_abort();
		// Though, you will never see this message. The kernel will just insta stall
		// As there's nothing to print this too.
		// And the lack of sse support will cause an error in the abort_msg
	}
	const char *vendor			   = get_cpuid_vendor();
	uint32_t	max_cpuid		   = get_cpuid_max();
	uint32_t	max_extended_cpuid = get_cpuid_max_extended();
	const char *brand_string	   = get_cpuid_brand_string();

	// ========================== INITIALIZING FPU (NEEDED for fpu optimized code)=========================
	struct fpu_features fpu_activated_features = init_fpu();

#define GRUB_FRAMEBUFFER
#if !defined(GRUB_FRAMEBUFFER) || defined(QEMU)
	initialize_terminal();
	terminal_set_scroll(0);
	kprintf("\n===========Terminal Initialized=============\n");
#endif

	kprintf("Brand string= %s\n", brand_string);
	kprintf("Max cpuid command= %h\n", max_extended_cpuid);
	kprintf("Cpuid Vendor = %s\n", vendor);
	kprintf("Max cpuid command= %h\n", max_cpuid);
	print_activated_fpu_features(fpu_activated_features);
	if (fpu_activated_features.fpu)
	{
		float a	  = 23.1;
		float b	  = 42.6;
		float res = fpu_add(a, b);
		kprintf("result of fpu addition: (%f + %f) = %f\n", a, b, res);
	}

	test_sse_alignment();

	kprintf("multiboot2 information addr = %h, magic = %h, is_proper_multiboot_32 = %u\n", mb2_info_addr, magic, is_proper_multiboot_32);
	bool is_proper_multiboot = is_proper_multiboot_32;
	if (is_proper_multiboot)
	{
		kprintf("It's proper multiboot\n");
	}
	else
	{
		abort_msg("Not proper multiboot\n");
		// The abbort might not be needed anyway.
	}

#ifndef DEBUG

#	define BIOS_FRAMEBUFFER_HACK
#	ifdef GRUB_FRAMEBUFFER
	struct framebuffer_info_t grub_fb_info = get_framebuffer(mb2_info_addr);

	uint8_t bpp	 = grub_fb_info.bit_per_pixel;
	bool	type = grub_fb_info.type;

	kprintf("FB addr: %h %h, pitch=%u, w=%u, h=%u, bpp=%u, type=%u\n", grub_fb_info.base_addr_high, grub_fb_info.base_addr_low,
		grub_fb_info.pitch, grub_fb_info.width, grub_fb_info.height, bpp, grub_fb_info.type);
	assert(bpp == 32, "Need 32 bit per pixel, else we are fucked!\n");
	assert(type == 1, "Need color support\n");

	uint32_t				 width		  = grub_fb_info.width;
	uint32_t				 height		  = grub_fb_info.height;
	uint32_t				 pitch		  = grub_fb_info.pitch;
	volatile struct color_t *base_address = (struct color_t *)grub_fb_info.base_addr_low;

#	elifdef BIOS_FRAMEBUFFER_HACK
	kprintf("in elif\n\n\n");
	uint32_t				 width		  = 1024;
	uint32_t				 height		  = 768;
	uint32_t				 pitch		  = 0;
	volatile struct color_t *base_address = (struct color_t *)0;

	uint16_t result = call_real_mode_function(add16_ref, width, height); // argc automatically calculated
	print_args16(&args16_start);
#	endif

#	define VIS_TEST
#	ifdef VIS_TEST
	do_test_c(base_address, width, height, pitch);
#	endif

#endif // DEBUG

#define GET_RSDP
#ifdef GET_RSDP
	// Eventually, learn to get rsdp from bios and uefi function calls
	struct rsdp_tagged_c rsdp_tagged	   = get_rsdp_grub(mb2_info_addr);
	char				*rsdp_type_names[] = {"NULL", "OLD", "NEW"};
	char				*name			   = rsdp_type_names[rsdp_tagged.new_or_old];
	void				*rsdp			   = rsdp_tagged.rsdp;
	kprintf("rsdp = %h, type=%s\n", rsdp, name);
#endif

	// init_paging();
	// init_page_bitmap();

	setup_interrupts(); // needed to have a working wait
	setup_keyboard();	// crash here

	// wait_test();

	to_real16_test();
	test_printf2();
	test_assert(); // gd, and set to false and play with it

	zig_main();
	terminal_writestring("\n====kernel main entering loop====\n");

	cpp_main();
	module_main();
	while (true)
	{
		// kernel main loop
		cpp_event_loop();

		wait(1.f / 60.f);
	}

	return;

	// int* big_array = (int*)kmalloc(1024 * 1024 * 1024);
	// for (int i = 0; i < 100000; i++) {
	// 	big_array[i] = i;
	// }
	//
	// print_array_terminal(&term, big_array, 100000);
}
