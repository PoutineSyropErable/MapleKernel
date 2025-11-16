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

#ifdef HAVE_IDTR_T
static inline void __lidt(idtr_t idt) {

	__asm__ volatile("lidt %0" : : "m"(idt));
}
#endif

static inline void __sti() {

	__asm__ volatile("sti"); // set the interrupt flag
}

static inline void __cli() {

	__asm__ volatile("cli"); // set the interrupt flag
}

static inline void __int(uint8_t int_vector) {
	__asm__ volatile("int %0" : : "i"(int_vector) : "memory");
}
