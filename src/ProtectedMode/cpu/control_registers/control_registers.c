#pragma once
#include "control_registers.h"
#include "static_assert.h"
#include <stdint.h>

#error "This file should never be compiled!"

static inline uint32_t _get_cr0(void)
{
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	return cr0;
}

static inline void _set_cr0(uint32_t cr0)
{
	asm volatile("mov %0, %%cr0" ::"r"(cr0));
}

static inline uint32_t _get_cr3(void)
{
	uint32_t cr3;
	asm volatile("mov %%cr3, %0" : "=r"(cr3));
	return cr3;
}

static inline void _set_cr3(uint32_t cr3)
{
	asm volatile("mov %0, %%cr3" ::"r"(cr3));
}

static inline uint32_t _get_cr4(void)
{
	uint32_t cr4;
	asm volatile("mov %%cr4, %0" : "=r"(cr4));
	return cr4;
}

static inline void _set_cr4(uint32_t cr4)
{
	asm volatile("mov %0, %%cr4" ::"r"(cr4));
}

// Helper functions to work with the structs
static inline struct cr0_t get_cr0(void)
{
	union
	{
		uint32_t	 raw;
		struct cr0_t bits;
	} cr0;

	cr0.raw = _get_cr0();
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
	_set_cr0(value.raw);
}

static inline struct cr3_t get_cr3(void)
{
	union
	{
		uint32_t	 raw;
		struct cr3_t bits;
	} cr3;

	cr3.raw = _get_cr3();
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
	_set_cr3(value.raw);
}

static inline struct cr4_t get_cr4(void)
{
	union
	{
		uint32_t	 raw;
		struct cr4_t bits;
	} cr4;

	cr4.raw = _get_cr4();
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
	_set_cr4(value.raw);
}
