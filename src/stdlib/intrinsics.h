#pragma once
#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

	static inline void __outb(uint16_t port, uint8_t val)
	{
		/*

		%0 = val : Value to output
		%1 = port: The io port
		*/
		__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
	}

	static inline void __outw(uint16_t port, uint16_t val)
	{
		/*

		%0 = val : Value to output
		%1 = port: The io port
		*/
		__asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
	}

	static inline uint8_t __inb(uint16_t port)
	{
		uint8_t ret;
		__asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
		return ret;
	}

	// idt.h has __sti

	static inline void __sti()
	{

		__asm__ volatile("sti"); // set the interrupt flag
	}

	static inline void __cli()
	{

		__asm__ volatile("cli"); // set the interrupt flag
	}

	// only works in 01 and more
	static inline void __int(const uint8_t int_vector)
	{
		__asm__ volatile("int %0" : : "i"(int_vector) : "memory");
	}

	static inline void __nop()
	{
		__asm__ volatile("nop");
	}

	static inline void __hlt()
	{
		__asm__ volatile("hlt");
	}

	// Read MSR into eax/edx
	static inline void read_msr(uint32_t msr_index, uint32_t *eax, uint32_t *edx)
	{
		__asm__ volatile("rdmsr"
			: "=a"(*eax), "=d"(*edx) // outputs
			: "c"(msr_index)		 // input
		);
	}

	// Write MSR from eax/edx
	static inline void write_msr(uint32_t msr_index, uint32_t eax, uint32_t edx)
	{
		__asm__ volatile("wrmsr" : : "c"(msr_index), "a"(eax), "d"(edx));
	}

#define __int_O0(n) __asm__ volatile("int %0" : : "i"(n) : "memory")

#ifdef __cplusplus
}
#endif
