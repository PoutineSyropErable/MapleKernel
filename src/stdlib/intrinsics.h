#pragma once
#include "stddef.h"
#include "stdint.h"

static inline void __outb(uint16_t port, uint8_t val) {
	/*

	%0 = val : Value to output
	%1 = port: The io port
	*/
	__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t __inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

// idt.h has __sti

static inline void __sti() {

	__asm__ volatile("sti"); // set the interrupt flag
}

static inline void __cli() {

	__asm__ volatile("cli"); // set the interrupt flag
}

// only works in 01 and more
static inline void __int(const uint8_t int_vector) {
	__asm__ volatile("int %0" : : "i"(int_vector) : "memory");
}

#define __int_O0(n) \
	__asm__ volatile("int %0" : : "i"(n) : "memory")
