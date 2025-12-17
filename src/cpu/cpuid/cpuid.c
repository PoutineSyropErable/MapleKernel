#include "cpuid.h"
#include <stdint.h>

// Temporary function. Assume return true;

uint32_t cpuid_supported_check()
{
	uint32_t result;

	asm volatile(".intel_syntax noprefix\n" // Switch to Intel syntax

				 "pushfd\n"							 // Save original EFLAGS
				 "pushfd\n"							 // Copy to modify
				 "xor dword ptr [esp], 0x00200000\n" // Flip ID bit (bit 21)
				 "popfd\n"							 // Load modified EFLAGS
				 "pushfd\n"							 // Push to check if flip worked
				 "pop eax\n"						 // Read modified EFLAGS
				 "xor eax, [esp]\n"					 // XOR with original EFLAGS
				 "popfd\n"							 // Restore original EFLAGS
				 "and eax, 0x00200000\n"			 // Mask only the ID bit
				 "mov %0, eax\n"					 // Store result into C variable

				 ".att_syntax prefix\n" // Switch back to AT&T syntax
		: "=r"(result)					// output
		:								// no input
		: "eax"							// clobbered register
	);

	return result;
}
