#pragma once
#include <stdint.h>

#include "crn.h"

namespace x86
{

// Control Register 0 (CR0)

// Extended Control Register 0 (XCR0) - Extended State Control
struct xcr0_t
{
	uint64_t x87 : 1;		 // x87 FPU state
	uint64_t sse : 1;		 // SSE state (XMM registers)
	uint64_t avx : 1;		 // AVX state (YMM registers)
	uint64_t bndreg : 1;	 // MPX Bounds registers
	uint64_t bndcsr : 1;	 // MPX Configuration/status
	uint64_t opmask : 1;	 // AVX-512 Opmask registers
	uint64_t zmm_hi256 : 1;	 // AVX-512 High 256 bits of ZMM0-15
	uint64_t hi16_zmm : 1;	 // AVX-512 High 16 ZMM registers (16-31)
	uint64_t reserved1 : 1;	 // Reserved
	uint64_t pkru : 1;		 // Protection Key Rights
	uint64_t reserved2 : 54; // Reserved
};

// Model Specific Register: Extended Feature Enable Register (EFER)
struct msr_efer_t
{
	uint64_t sce : 1;		 // System Call Extensions (SYSCALL/SYSRET)
	uint64_t reserved1 : 7;	 // Reserved
	uint64_t lme : 1;		 // Long Mode Enable
	uint64_t reserved2 : 1;	 // Reserved
	uint64_t lma : 1;		 // Long Mode Active (read-only)
	uint64_t nxe : 1;		 // No-Execute Enable
	uint64_t svme : 1;		 // Secure Virtual Machine Enable
	uint64_t lmsle : 1;		 // Long Mode Segment Limit Enable
	uint64_t ffxsr : 1;		 // Fast FXSAVE/FXRSTOR
	uint64_t tce : 1;		 // Translation Cache Extension
	uint64_t reserved3 : 48; // Reserved
};

// FPU Control Word
struct fpu_control_word_t
{
	uint16_t invalid_operation : 1;
	uint16_t denormal_operand : 1;
	uint16_t zero_divide : 1;
	uint16_t overflow : 1;
	uint16_t underflow : 1;
	uint16_t precision : 1;
	uint16_t reserved1 : 2;
	uint16_t precision_control : 2; // 00=24bit, 01=reserved, 10=53bit, 11=64bit
	uint16_t rounding_control : 2;	// 00=nearest, 01=down, 10=up, 11=truncate
	uint16_t infinity_control : 1;	// 0=projective, 1=affine (only 387+)
	uint16_t reserved2 : 3;
};

// MXCSR Register (SSE Control/Status)
struct mxcsr_t
{
	uint32_t invalid_operation : 1;
	uint32_t denormal_operand : 1;
	uint32_t zero_divide : 1;
	uint32_t overflow : 1;
	uint32_t underflow : 1;
	uint32_t precision : 1;
	uint32_t denormals_are_zeros : 1;
	uint32_t invalid_operation_mask : 1;
	uint32_t denormal_operand_mask : 1;
	uint32_t zero_divide_mask : 1;
	uint32_t overflow_mask : 1;
	uint32_t underflow_mask : 1;
	uint32_t precision_mask : 1;
	uint32_t rounding_control : 2; // 00=nearest, 01=down, 10=up, 11=truncate
	uint32_t flush_to_zero : 1;
	uint32_t reserved : 16;
};

// Helper class to manipulate control registers
class ControlRegisters
{
  public:
	// Read control registers
	static cr0_t read_cr0()
	{
		uint32_t value;
		asm volatile("mov %%cr0, %0" : "=r"(value));
		return *reinterpret_cast<cr0_t *>(&value);
	}

	static cr4_t read_cr4()
	{
		uint32_t value;
		asm volatile("mov %%cr4, %0" : "=r"(value));
		return *reinterpret_cast<cr4_t *>(&value);
	}

	// Write control registers
	static void write_cr0(const cr0_t &cr0)
	{
		uint32_t value = *reinterpret_cast<const uint32_t *>(&cr0);
		asm volatile("mov %0, %%cr0" ::"r"(value));
	}

	static void write_cr4(const cr4_t &cr4)
	{
		uint32_t value = *reinterpret_cast<const uint32_t *>(&cr4);
		asm volatile("mov %0, %%cr4" ::"r"(value));
	}

	// Enable x87 FPU (simple version)
	static void enable_x87_fpu()
	{
		cr0_t cr0 = read_cr0();
		cr0.em	  = 0; // Clear EM: use native FPU
		cr0.mp	  = 1; // Set MP: monitor co-processor
		cr0.ts	  = 0; // Clear TS: not task switched
		cr0.ne	  = 1; // Set NE: enable native exceptions
		write_cr0(cr0);

		// Initialize FPU
		asm volatile("fninit");

		// Set default control word (double precision, round nearest)
		uint16_t cw = 0x037F;
		asm volatile("fldcw %0" ::"m"(cw));
	}

	// Enable SSE/SSE2
	static void enable_sse()
	{
		cr4_t cr4	   = read_cr4();
		cr4.osfxsr	   = 1; // Enable FXSAVE/FXRSTOR (required for SSE)
		cr4.osxmmexcpt = 1; // Enable SSE exceptions
		write_cr4(cr4);

		// Set default MXCSR
		uint32_t mxcsr = 0x1F80; // Default: all exceptions masked
		asm volatile("ldmxcsr %0" ::"m"(mxcsr));
	}

	// Enable AVX (requires XSAVE support)
	static void enable_avx()
	{
		// First enable SSE (prerequisite)
		enable_sse();

		// Enable XSAVE in CR4
		cr4_t cr4	= read_cr4();
		cr4.osxsave = 1; // Enable XSAVE/XRSTOR instructions
		write_cr4(cr4);

		// Enable AVX state in XCR0
		uint64_t xcr0;
		asm volatile("xgetbv" : "=a"(xcr0) : "c"(0) : "edx");
		xcr0 |= (1 << 1); // Enable SSE state
		xcr0 |= (1 << 2); // Enable AVX state
		asm volatile("xsetbv" ::"c"(0), "a"(xcr0), "d"(xcr0 >> 32));
	}

	// Check if SSE is available
	static bool has_sse()
	{
		// Check CPUID feature flags
		uint32_t eax, ebx, ecx, edx;
		asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1), "c"(0));
		return (edx >> 25) & 1; // SSE bit in EDX
	}

	// Check if AVX is available
	static bool has_avx()
	{
		// Check CPUID feature flags
		uint32_t eax, ebx, ecx, edx;
		asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1), "c"(0));

		// Need OSXSAVE, XSAVE, and AVX bits
		bool has_osxsave = (ecx >> 27) & 1;
		bool has_xsave	 = (ecx >> 26) & 1;
		bool has_avx_bit = (ecx >> 28) & 1;

		if (!has_osxsave || !has_xsave || !has_avx_bit)
			return false;

		// Check if OS has enabled AVX in XCR0
		uint64_t xcr0;
		asm volatile("xgetbv" : "=a"(xcr0) : "c"(0) : "edx");
		return (xcr0 & (1 << 2)) != 0; // AVX state enabled
	}

	// Read FPU control word
	static fpu_control_word_t read_fpu_cw()
	{
		uint16_t value;
		asm volatile("fstcw %0" : "=m"(value));
		return *reinterpret_cast<fpu_control_word_t *>(&value);
	}

	// Write FPU control word
	static void write_fpu_cw(const fpu_control_word_t &cw)
	{
		uint16_t value = *reinterpret_cast<const uint16_t *>(&cw);
		asm volatile("fldcw %0" ::"m"(value));
	}

	// Read MXCSR
	static mxcsr_t read_mxcsr()
	{
		uint32_t value;
		asm volatile("stmxcsr %0" : "=m"(value));
		return *reinterpret_cast<mxcsr_t *>(&value);
	}

	// Write MXCSR
	static void write_mxcsr(const mxcsr_t &mxcsr)
	{
		uint32_t value = *reinterpret_cast<const uint32_t *>(&mxcsr);
		asm volatile("ldmxcsr %0" ::"m"(value));
	}

	// Save FPU/SSE/AVX state (simplified - real version needs proper buffer)
	static void save_fpu_state(void *buffer)
	{
		if (has_avx())
		{
			asm volatile("xsave (%0)" ::"r"(buffer) : "memory");
		}
		else if (has_sse())
		{
			asm volatile("fxsave (%0)" ::"r"(buffer) : "memory");
		}
		else
		{
			asm volatile("fsave (%0)" ::"r"(buffer) : "memory");
		}
	}

	// Restore FPU/SSE/AVX state
	static void restore_fpu_state(void *buffer)
	{
		if (has_avx())
		{
			asm volatile("xrstor (%0)" ::"r"(buffer) : "memory");
		}
		else if (has_sse())
		{
			asm volatile("fxrstor (%0)" ::"r"(buffer) : "memory");
		}
		else
		{
			asm volatile("frstor (%0)" ::"r"(buffer) : "memory");
		}
	}
};

// Pre-defined values for common configurations
namespace defaults
{
constexpr fpu_control_word_t fpu_cw = {.invalid_operation = 0,
	.denormal_operand									  = 0,
	.zero_divide										  = 0,
	.overflow											  = 0,
	.underflow											  = 0,
	.precision											  = 0,
	.reserved1											  = 0,
	.precision_control									  = 3, // 64-bit precision
	.rounding_control									  = 0, // Round to nearest
	.infinity_control									  = 0, // Projective infinity
	.reserved2											  = 0};

constexpr mxcsr_t mxcsr = {.invalid_operation = 0,
	.denormal_operand						  = 0,
	.zero_divide							  = 0,
	.overflow								  = 0,
	.underflow								  = 0,
	.precision								  = 0,
	.denormals_are_zeros					  = 0,
	.invalid_operation_mask					  = 1,
	.denormal_operand_mask					  = 1,
	.zero_divide_mask						  = 1,
	.overflow_mask							  = 1,
	.underflow_mask							  = 1,
	.precision_mask							  = 1,
	.rounding_control						  = 0, // Round to nearest
	.flush_to_zero							  = 0,
	.reserved								  = 0};
} // namespace defaults

} // namespace x86
