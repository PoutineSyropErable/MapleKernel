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

struct cpuid_reg _cpuid(uint32_t function, uint32_t subleaf)
{

	struct cpuid_reg regs;
	asm volatile(".intel_syntax noprefix\n" // Switch to Intel syntax

				 // mov eax, function
				 // mov ecx, subleaf

				 "cpuid\n"				// Call CPUID
				 ".att_syntax prefix\n" // Switch back to AT&T syntax
		: "=a"(regs.eax),				// output: EAX
		"=b"(regs.ebx),					// output: EBX
		"=c"(regs.ecx),					// output: ECX
		"=d"(regs.edx)					// output: EDX
		: "a"(function),				// EAX input: CPUID function
		"c"(subleaf)					// ECX input: subleaf
	);

	return regs;
}
