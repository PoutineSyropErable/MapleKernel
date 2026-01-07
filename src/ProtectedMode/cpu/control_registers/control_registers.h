#pragma once
#include "static_assert.h"
#include <stdint.h>

struct cr0_t

{
	uint32_t pe : 1;		 // Protected Mode Enable
	uint32_t mp : 1;		 // Monitor coProcessor (x87 FPU)
	uint32_t em : 1;		 // Emulation (x87 FPU)
	uint32_t ts : 1;		 // Task Switched (x87 FPU)
	uint32_t et : 1;		 // Extension Type (387 vs. 287)
	uint32_t ne : 1;		 // Numeric Error (x87 exceptions)
	uint32_t reserved1 : 10; // Reserved
	uint32_t wp : 1;		 // Write Protect
	uint32_t reserved2 : 1;	 // Reserved
	uint32_t am : 1;		 // Alignment Mask
	uint32_t reserved3 : 10; // Reserved
	uint32_t nw : 1;		 // Not Write-through
	uint32_t cd : 1;		 // Cache Disable
	uint32_t pg : 1;		 // Paging Enable
};

// Control Register 3 (CR3) - Page Directory Base Register
struct cr3_t
{
};

// Control Register 4 (CR4) - Extended Features
struct cr4_t
{
	uint32_t vme : 1;		 // Virtual-8086 Mode Extensions
	uint32_t pvi : 1;		 // Protected-Mode Virtual Interrupts
	uint32_t tsd : 1;		 // Time Stamp Disable
	uint32_t de : 1;		 // Debugging Extensions
	uint32_t pse : 1;		 // Page Size Extension (4MB pages)
	uint32_t pae : 1;		 // Physical Address Extension
	uint32_t mce : 1;		 // Machine Check Enable
	uint32_t pge : 1;		 // Page Global Enable
	uint32_t pce : 1;		 // Performance Monitoring Counter Enable
	uint32_t osfxsr : 1;	 // OS Support for FXSAVE/FXRSTOR (SSE/SSE2)
	uint32_t osxmmexcpt : 1; // OS Support for Unmasked SIMD FP Exceptions
	uint32_t umip : 1;		 // User-Mode Instruction Prevention
	uint32_t reserved1 : 1;	 // Reserved
	uint32_t fsgsbase : 1;	 // FS/GS Base Access
	uint32_t pcide : 1;		 // PCID Enable
	uint32_t osxsave : 1;	 // OS Support for XSAVE/XRSTOR (AVX/AVX512)
	uint32_t reserved2 : 1;	 // Reserved
	uint32_t smep : 1;		 // Supervisor Mode Execution Prevention
	uint32_t smap : 1;		 // Supervisor Mode Access Prevention
	uint32_t pke : 1;		 // Protection Key Enable
	uint32_t reserved3 : 10; // Reserved
};

// ==================== CR0 ====================
static inline struct cr0_t get_cr0(void)
{
	union
	{
		uint32_t	 raw;
		struct cr0_t bits;
	} cr0;

	asm volatile("mov %%cr0, %0" : "=r"(cr0.raw));
	return cr0.bits;
}

static inline void set_cr0(struct cr0_t cr0)
{
	union
	{
		uint32_t	 raw;
		struct cr0_t bits;
	} value;

	value.bits = cr0;
	asm volatile("mov %0, %%cr0" : : "r"(value.raw));
}

// ==================== CR3 ====================
static inline struct cr3_t get_cr3(void)
{
	union
	{
		uint32_t	 raw;
		struct cr3_t bits;
	} cr3;

	asm volatile("mov %%cr3, %0" : "=r"(cr3.raw));
	return cr3.bits;
}

static inline void set_cr3(struct cr3_t cr3)
{
	union
	{
		uint32_t	 raw;
		struct cr3_t bits;
	} value;

	value.bits = cr3;
	asm volatile("mov %0, %%cr3" : : "r"(value.raw));
}

// ==================== CR4 ====================
static inline struct cr4_t get_cr4(void)
{
	union
	{
		uint32_t	 raw;
		struct cr4_t bits;
	} cr4;

	asm volatile("mov %%cr4, %0" : "=r"(cr4.raw));
	return cr4.bits;
}

static inline void set_cr4(struct cr4_t cr4)
{
	union
	{
		uint32_t	 raw;
		struct cr4_t bits;
	} value;

	value.bits = cr4;
	asm volatile("mov %0, %%cr4" : : "r"(value.raw));
}
