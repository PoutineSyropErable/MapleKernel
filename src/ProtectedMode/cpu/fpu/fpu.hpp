#pragma once
#include "static_assert.h"
#include <stddef.h>
#include <stdint.h>

namespace fpu
{

struct cpuid_fpu_1_ecx
{
	// Bit 0-1: FPU-related features
	uint32_t sse3 : 1;		// Streaming SIMD Extensions 3
	uint32_t pclmulqdq : 1; // PCLMULQDQ instruction (uses XMM)

	// Bit 2-8: Non-FPU features
	uint32_t others1 : 7; // dtes64, monitor, ds_cpl, vmx, smx, est, tm2

	// Bit 9: FPU-related feature
	uint32_t ssse3 : 1; // Supplemental Streaming SIMD Extensions 3

	// Bit 10-12: Mixed
	uint32_t others2 : 2; // cid, sdbg (non-FPU)
	uint32_t fma : 1;	  // FMA instructions (FPU!)

	// Bit 13-18: Non-FPU features
	uint32_t others3 : 6; // cx16, xtpr, pdcm, reserved, pcid, dca

	// Bit 19-20: FPU-related features
	uint32_t sse4_1 : 1; // SSE4.1
	uint32_t sse4_2 : 1; // SSE4.2

	// Bit 21-25: Mixed
	uint32_t others4 : 3; // x2apic, movbe, popcnt (non-FPU)
	uint32_t others5 : 1; // tsc_deadline (non-FPU)
	uint32_t aes : 1;	  // AES instructions (uses XMM)

	// Bit 26-27: FPU state management
	uint32_t xsave : 1;	  // XSAVE/XSTOR - FPU state save
	uint32_t osxsave : 1; // OSXSAVE - OS support for XSAVE

	// Bit 28-29: FPU-related features
	uint32_t avx : 1;  // AVX - Advanced Vector Extensions
	uint32_t f16c : 1; // F16C - half-precision convert

	// Bit 30-31: Non-FPU features
	uint32_t others6 : 2; // rdrand, hypervisor
};
STATIC_ASSERT(sizeof(cpuid_fpu_1_ecx) == sizeof(uint32_t), "must be 32 bit\n");

struct cpuid_fpu_1_edx
{
	uint32_t fpu : 1;	   // x87 FPU on-chip
	uint32_t others1 : 22; // everything else not directly FPU/MMX/SSE related
	uint32_t mmx : 1;	   // MMX instruction set
	uint32_t others2 : 1;  // reserved/other
	uint32_t sse : 1;	   // SSE
	uint32_t sse2 : 1;	   // SSE2
	uint32_t others3 : 5;  // remaining reserved/other bits
};

STATIC_ASSERT(sizeof(cpuid_fpu_1_edx) == sizeof(uint32_t), "must be 32 bit\n");

struct cpuid_fpu_7_0_ebx
{
	uint32_t fsgsbase : 1;	 // FS/GS base access
	uint32_t sgx : 1;		 // Intel SGX
	uint32_t bmi1 : 1;		 // Bit Manipulation Instruction Set 1
	uint32_t hle : 1;		 // Hardware Lock Elision
	uint32_t avx2 : 1;		 // AVX2 instructions (FPU!)
	uint32_t others1 : 2;	 // reserved/FDP deprecation
	uint32_t bmi2 : 1;		 // Bit Manipulation Instruction Set 2
	uint32_t erms : 1;		 // Enhanced REP MOVSB/STOSB
	uint32_t invpcid : 1;	 // INVPCID instruction
	uint32_t rtm : 1;		 // Restricted Transactional Memory
	uint32_t rtm_m : 1;		 // RTM always in TSX? (depends)
	uint32_t mpx : 1;		 // Memory Protection Extensions
	uint32_t avx512f : 1;	 // AVX-512 Foundation (FPU!)
	uint32_t avx512dq : 1;	 // AVX-512 DQ (Double & Quadword) (FPU!)
	uint32_t rdseed : 1;	 // RDSEED instruction
	uint32_t adx : 1;		 // Multi-precision Add-Carry
	uint32_t smap : 1;		 // Supervisor Mode Access Prevention
	uint32_t avx512ifma : 1; // AVX-512 Integer FMA (FPU!)
	uint32_t others2 : 3;	 // reserved
	uint32_t clflushopt : 1; // CLFLUSHOPT instruction
	uint32_t clwb : 1;		 // CLWB instruction
	uint32_t intel_pt : 1;	 // Intel Processor Trace
	uint32_t avx512pf : 1;	 // AVX-512 Prefetch (FPU!)
	uint32_t avx512er : 1;	 // AVX-512 Exponential & Reciprocal (FPU!)
	uint32_t avx512cd : 1;	 // AVX-512 Conflict Detection (FPU!)
	uint32_t sha : 1;		 // SHA extensions
	uint32_t avx512bw : 1;	 // AVX-512 Byte & Word (FPU!)
	uint32_t avx512vl : 1;	 // AVX-512 Vector Length (FPU!)
};
STATIC_ASSERT(sizeof(cpuid_fpu_7_0_ebx) == sizeof(uint32_t), "must be 32 bit\n");

struct cpuid_fpu_7_0_ecx
{
	uint32_t others1 : 1;			 // reserved
	uint32_t avx512vbmi : 1;		 // AVX-512 Vector Bit Manipulation (FPU!)
	uint32_t others2 : 1;			 // reserved
	uint32_t avx512vbmi2 : 1;		 // AVX-512 VBMI2 (FPU!)
	uint32_t others3 : 1;			 // reserved
	uint32_t avx512vnni : 1;		 // AVX-512 Vector Neural Network (FPU!)
	uint32_t avx512bitalg : 1;		 // AVX-512 Bitalg (FPU!)
	uint32_t others4 : 1;			 // reserved
	uint32_t avx512vpopcntdq : 1;	 // AVX-512 VPOPCNTDQ (FPU!)
	uint32_t others5 : 6;			 // reserved/misc
	uint32_t avx512vp2intersect : 1; // AVX-512 VP2INTERSECT (FPU!)
	uint32_t others6 : 15;			 // remaining bits
};
STATIC_ASSERT(sizeof(cpuid_fpu_7_0_ecx) == sizeof(uint32_t), "must be 32 bit\n");

struct cpuid_7_0_edx
{
	uint32_t others1 : 2;		// reserved
	uint32_t avx512_4vnniw : 1; // AVX-512 4-register Neural Net (FPU!)
	uint32_t avx512_4fmaps : 1; // AVX-512 4-register Multiply Accum (FPU!)
	uint32_t others2 : 28;		// remaining bits
};
STATIC_ASSERT(sizeof(cpuid_7_0_edx) == sizeof(uint32_t), "must be 32 bit\n");

} // namespace fpu
