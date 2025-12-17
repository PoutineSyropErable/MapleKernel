#include "assert.h"
#include "control_registers.h"
#include "cpuid.hpp"
#include "fpu.h"
#include "fpu.hpp"
#include "string.h"

using namespace fpu;

extern "C" struct fpu_features init_fpu()
{

	struct fpu_features ret;
	memset(&ret, 0, sizeof(fpu_features));

	cr0_t cr0 = get_cr0();
	cr3_t cr3 = get_cr3();
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
	union
	{
		struct cpuid_reg  raw_regs;
		struct fpu_1_regs typed_regs;
	} regs_uts;

	regs_uts.raw_regs			   = cpuid_1.regs;
	fpu_1_regs fpu_cpuid1_features = regs_uts.typed_regs;
	if (fpu_cpuid1_features.fpu_edx.fpu)
	{
		ret.fpu = true;
		cr0.mp	= true;
		cr0.em	= false;
		cr0.ts	= false;

		asm volatile("fninit"); // Initialize FPU
		set_cr0(cr0);
	}

	if (fpu_cpuid1_features.fpu_edx.mmx)
	{
		ret.mmx = true;
	}

	if (fpu_cpuid1_features.fpu_edx.sse || fpu_cpuid1_features.fpu_edx.sse2)
	{
		cr4.osfxsr	   = 1;
		cr4.osxmmexcpt = 1;
		ret.sse		   = fpu_cpuid1_features.fpu_edx.sse;
		ret.sse2	   = fpu_cpuid1_features.fpu_edx.sse2;
		set_cr4(cr4);
		uint32_t mxcsr = 0x1f80; // default mask
		asm volatile("ldmxcsr %0" : : "m"(mxcsr));
	}

	if (fpu_cpuid1_features.fpu_ecx.avx && fpu_cpuid1_features.fpu_ecx.osxsave)
	{
		cr4.osxsave = 1;
		set_cr4(cr4);

		uint32_t eax, edx;
		asm volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
		eax |= (1 << 1) | (1 << 2); // enable XMM and YMM state
		asm volatile("xsetbv" : : "a"(eax), "d"(edx), "c"(0));

		ret.avx = true;
	}

	set_cr0(cr0);
	set_cr3(cr3);
	set_cr4(cr4);
	return ret;
}
