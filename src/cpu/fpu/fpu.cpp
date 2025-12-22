#include "assert.h"
#include "cast.h"
#include "control_registers.h"
#include "cpuid.hpp"
#include "fpu.h"
#include "fpu.hpp"
#include "string.h"

using namespace fpu;

extern "C" void print_activated_fpu_features(const struct fpu_features fpu_features)
{
	const struct fpu_features &ff = fpu_features;
	kprintf("\n-----Activated FPU Features-----\n");
	kprintf("fpu   		= %b\n", ff.fpu);
	kprintf("mmx   		= %b\n", ff.mmx);
	kprintf("sse   		= %b\n", ff.sse);
	kprintf("sse2  		= %b\n", ff.sse2);
	kprintf("sse3  		= %b\n", ff.sse3);
	kprintf("sse41 		= %b\n", ff.sse41);
	kprintf("sse42 		= %b\n", ff.sse42);
	kprintf("avx_256   	= %b\n", ff.avx_256);
	kprintf("xsave 		= %b\n", ff.xsave);

	kprintf("avx_512   	= %b\n", ff.avx_512);
	kprintf("avx2 		= %b\n", ff.avx2);
	kprintf("\n");
}

extern "C" struct fpu_features init_fpu()
{

	struct fpu_features ret;
	memset(&ret, 0, sizeof(fpu_features));

	cr0_t cr0 = get_cr0();
	// CR3 isn't used in FPU
	cr4_t cr4 = get_cr4();

	struct cpuid::cpuid_verified_result cpuid_1 = cpuid::call_cpuid(cpuid::CpuidFunction::ProcessorInfo, {.raw = 0});
	if (cpuid_1.has_error())
	{
		abort_msg("If we can't even get cpuid1, we are fucked anyway\n");
	}
	struct fpu_1_regs
	{
		uint32_t		unused;
		uint32_t		unused2;
		cpuid_fpu_1_ecx fpu_ecx;
		cpuid_fpu_1_edx fpu_edx;
	};

	fpu_1_regs fpu_cpuid1_features = BITCAST(struct fpu_1_regs, cpuid_1.regs);
	if (fpu_cpuid1_features.fpu_edx.fpu)
	{
		ret.fpu = true;

		cr0.mp = true;
		cr0.em = false;
		cr0.ts = false;
		set_cr0(cr0);

		asm volatile("fninit"); // Initialize FPU

		// Set default control word (double precision, round nearest)
		uint16_t fpu_cw = 0x037F;
		asm volatile("fldcw %0" : : "m"(fpu_cw));
	}

	if (fpu_cpuid1_features.fpu_edx.mmx)
	{
		ret.mmx = true;
	}

	if (fpu_cpuid1_features.fpu_edx.sse || fpu_cpuid1_features.fpu_edx.sse2)
	{

		// Enable SSE features in CR4
		cr4.osfxsr	   = 1; // Enable FXSAVE/FXRSTOR instructions
		cr4.osxmmexcpt = 1; // Enable unmasked SSE exceptions

		set_cr4(cr4);

		// Set default MXCSR (SSE control/status register)
		uint32_t mxcsr = 0x1F80; // All exceptions masked
		asm volatile("ldmxcsr %0" : : "m"(mxcsr));

		ret.sse	 = fpu_cpuid1_features.fpu_edx.sse;
		ret.sse2 = fpu_cpuid1_features.fpu_edx.sse2;
	}

	// --- SSE3/SSSE3/SSE4.1/SSE4.2 ---
	// These are enabled automatically when SSE is enabled
	if (fpu_cpuid1_features.fpu_ecx.sse3)
	{
		ret.sse3 = true; // nothing else needed
	}

	if (fpu_cpuid1_features.fpu_ecx.ssse3)
	{
		ret.ssse3 = true; // no extra CR4 or MXCSR needed
	}

	if (fpu_cpuid1_features.fpu_ecx.sse4_1)
	{
		ret.sse41 = true; // just mark it
	}

	if (fpu_cpuid1_features.fpu_ecx.sse4_2)
	{
		ret.sse42 = true; // just mark it
	}

	if (fpu_cpuid1_features.fpu_ecx.avx && fpu_cpuid1_features.fpu_ecx.osxsave)
	{

		// Enable xsave
		ret.xsave	= true;
		cr4.osxsave = 1;
		set_cr4(cr4);

		// Enable avx_256. (TODO, learn about xgetbv and xsetbv. This little bit of code is vibe coded)
		ret.avx_256 = true;
		uint32_t eax, edx;
		asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
		eax |= (1 << 1) | (1 << 2); // enable XMM and YMM state
		asm volatile("xsetbv" : : "a"(eax), "d"(edx), "c"(0));

		// Optional: Check if OS has actually enabled it
		asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
		if (!(eax & (1 << 2)))
		{
			ret.avx_256 = false; // OS didn't enable it
			kprintf("AVX-256 wasn't actually enabled though it tried\n");
		}
	}

	// --- AVX2 & AVX512 (optional - check CPUID function 7) ---
// #define ENABLE_ADVANCED_FEATURES
#ifdef ENABLE_ADVANCED_FEATURES
	// This won't work on the current QEMU, as it doesn't support these features
	// And, xcr0 is needed anyway.
	abort_msg("Read the comment above. Go learn about fpu before you return here!\n");
	if (!ret.avx_256)
	{
		set_cr0(cr0);
		set_cr3(cr3);
		set_cr4(cr4);
		return ret;
	}

	// Check for AVX2 (CPUID function 7, subleaf 0, EBX bit 5)
	struct cpuid::cpuid_verified_result cpuid_7 = cpuid::call_cpuid(cpuid::CpuidFunction::ExtendedFeatures, {.raw = 0});

	if (cpuid_7.has_error())
	{

		kprintf("Warning, AVX-512 can't be supported, since this cpuid (CPUID function 7) isn't supported\n");
		set_cr0(cr0);
		set_cr4(cr4);
		return ret;
	}
	union
	{
		struct cpuid_reg raw;
		struct
		{
			uint32_t eax;
			uint32_t ebx;
			uint32_t ecx;
			uint32_t edx;
		} regs;
	} func7;

	func7.raw = cpuid_7.regs;

	// Check for AVX2
	if (func7.regs.ebx & (1 << 5))
	{
		ret.avx2 = true;
		// AVX2 uses same XCR0 settings as AVX
	}

	// Check for AVX512 foundation
	if (func7.regs.ebx & (1 << 16))
	{
		ret.avx_512 = true;
		// Need to enable additional bits in XCR0
		uint32_t eax, edx;
		asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
		eax |= (1 << 5) | (1 << 6) | (1 << 7); // Opmask, ZMM_hi256, Hi16_ZMM
		asm volatile("xsetbv" : : "a"(eax), "d"(edx), "c"(0));
	}

#endif

end:
	set_cr0(cr0);
	set_cr4(cr4);
	return ret;
}
