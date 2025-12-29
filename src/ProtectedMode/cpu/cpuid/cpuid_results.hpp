#pragma once
#include "static_assert.h"
#include <stddef.h>
#include <stdint.h>

// ECX returned by CPUID with EAX=1 : Basic Features
struct cpuid_basic_ecx
{
	uint32_t sse3 : 1;
	uint32_t pclmulqdq : 1;
	uint32_t dtes64 : 1;
	uint32_t monitor : 1;
	uint32_t ds_cpl : 1;
	uint32_t vmx : 1;
	uint32_t smx : 1;
	uint32_t est : 1;
	uint32_t tm2 : 1;
	uint32_t ssse3 : 1;
	uint32_t reserved1 : 1;
	uint32_t cnxt_id : 1;
	uint32_t sdbg : 1;
	uint32_t fma : 1;
	uint32_t cx16 : 1;
	uint32_t xtpr : 1;
	uint32_t pdcm : 1;
	uint32_t reserved2 : 1;
	uint32_t pcid : 1;
	uint32_t dca : 1;
	uint32_t sse4_1 : 1;
	uint32_t sse4_2 : 1;
	uint32_t x2apic : 1;
	uint32_t movbe : 1;
	uint32_t popcnt : 1;
	uint32_t tsc_deadline : 1;
	uint32_t aes : 1;
	uint32_t xsave : 1;
	uint32_t osxsave : 1;
	uint32_t avx : 1;
	uint32_t f16c : 1;
	uint32_t rdrand : 1;
};
STATIC_ASSERT(sizeof(cpuid_basic_ecx) == sizeof(uint32_t), "ECX must be 32-bit");

// EDX returned by CPUID with EAX=1 : Basic Features
struct cpuid_basic_edx
{
	uint32_t fpu : 1;		// x87 FPU on-chip
	uint32_t vme : 1;		// Virtual-8086 mode enhancements
	uint32_t de : 1;		// Debugging extensions
	uint32_t pse : 1;		// Page size extension
	uint32_t tsc : 1;		// Time Stamp Counter
	uint32_t msr : 1;		// Model-specific registers
	uint32_t pae : 1;		// Physical Address Extension
	uint32_t mce : 1;		// Machine Check Exception
	uint32_t cx8 : 1;		// CMPXCHG8 instruction
	uint32_t apic : 1;		// APIC on-chip.
	uint32_t reserved1 : 1; // reserved
	uint32_t sep : 1;		// SYSENTER/SYSEXIT
	uint32_t mtrr : 1;		// Memory Type Range Registers
	uint32_t pge : 1;		// Page Global Enable
	uint32_t mca : 1;		// Machine Check Architecture
	uint32_t cmov : 1;		// Conditional move instructions
	uint32_t pat : 1;		// Page Attribute Table
	uint32_t pse36 : 1;		// 36-bit page size extension
	uint32_t psn : 1;		// Processor serial number
	uint32_t clfsh : 1;		// CLFLUSH instruction
	uint32_t reserved2 : 1; // reserved
	uint32_t ds : 1;		// Debug store
	uint32_t acpi : 1;		// Thermal monitor
	uint32_t mmx : 1;		// MMX instruction set
	uint32_t fxsr : 1;		// FXSAVE/FXRSTOR
	uint32_t sse : 1;		// SSE
	uint32_t sse2 : 1;		// SSE2
	uint32_t ss : 1;		// Self-snoop
	uint32_t htt : 1;		// Hyper-threading
	uint32_t tm : 1;		// Thermal monitor
	uint32_t reserved3 : 1; // reserved
};
STATIC_ASSERT(sizeof(cpuid_basic_edx) == sizeof(uint32_t), "EDX must be 32-bit");
