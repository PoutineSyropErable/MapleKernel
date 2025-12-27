#pragma once
#include "assert.h"
#include "std.hpp"
#include <stdint.h>

// REQUIRES: -fno-strict-aliasing
// We can't apply it to just the header, that makes optimization completly die

namespace std
{

// #define DEBUG

// Wrapper for a pointer that can only be set once

// ======================================================================
// Modifiable (but private) MMIO pointer (This is the one you should use)
// ======================================================================
template <typename T> struct mmio_ptr
{
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "mmio_ptr only supports 1, 2, or 4 byte types");

	static constexpr bool T_is_uintX_t = is_same<T, uint8_t>::value || is_same<T, uint16_t>::value || is_same<T, uint32_t>::value;

  private:
	using underlying_t =
		typename conditional<sizeof(T) == 1, uint8_t, typename conditional<sizeof(T) == 2, uint16_t, uint32_t>::type>::type;

	const uintptr_t addr; // store address as integer

  public:
	using value_type = T;

	// constexpr constructor
	constexpr explicit mmio_ptr(uintptr_t a) : addr(a)
	{
	}
	constexpr explicit mmio_ptr(volatile T *p) : addr((uintptr_t)p)
	{
	}

	T read() const
	{
		volatile T	*p	 = reinterpret_cast<volatile T *>(addr);
		underlying_t raw = *(volatile underlying_t *)p;
		T			 val;
		__builtin_memcpy(&val, &raw, sizeof(T));
		return val;
	}

	void write(T v) const
	{
		volatile T	*p	 = reinterpret_cast<volatile T *>(addr);
		underlying_t raw = 0;
		__builtin_memcpy(&raw, &v, sizeof(T));
		*(volatile underlying_t *)p = raw;
	}
};

template <typename T> struct mmio_ptr_ro
{
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "mmio_ptr only supports 1, 2, or 4 byte types");

	static constexpr bool T_is_uintX_t = is_same<T, uint8_t>::value || is_same<T, uint16_t>::value || is_same<T, uint32_t>::value;

  private:
	using underlying_t =
		typename conditional<sizeof(T) == 1, uint8_t, typename conditional<sizeof(T) == 2, uint16_t, uint32_t>::type>::type;

	const uintptr_t addr; // store address as integer

  public:
	using value_type = T;

	// construct from raw pointer
	constexpr explicit mmio_ptr_ro(uintptr_t a) : addr(a)
	{
	}
	constexpr explicit mmio_ptr_ro(volatile T *p) : addr((uintptr_t)p)
	{
	}

	T read() const
	{
		volatile T *p	= reinterpret_cast<volatile T *>(addr);
		uint32_t	raw = *(volatile underlying_t *)p; // FULL register read
		T			val;
		__builtin_memcpy(&val, &raw, sizeof(T)); // copy into struct
		return val;
	}
};

template <typename T> struct mmio_ptr_wo
{
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "mmio_ptr only supports 1, 2, or 4 byte types");

	static constexpr bool T_is_uintX_t = is_same<T, uint8_t>::value || is_same<T, uint16_t>::value || is_same<T, uint32_t>::value;

  private:
	using underlying_t =
		typename conditional<sizeof(T) == 1, uint8_t, typename conditional<sizeof(T) == 2, uint16_t, uint32_t>::type>::type;

	// volatile T *const ptr; // mutable pointer. But never modified
	const uintptr_t addr; // store address as integer

  public:
	using value_type = T;

	constexpr explicit mmio_ptr_wo(volatile T *p) : addr((uintptr_t)p)
	{
	}

	// construct from raw pointer
	constexpr explicit mmio_ptr_wo(uintptr_t a) : addr(a)
	{
	}

	void write(const T v) const
	{
		volatile T	*ptr = reinterpret_cast<volatile T *>(addr);
		underlying_t raw = 0;
		__builtin_memcpy(&raw, &v, sizeof(T)); // copy struct → raw
		*(volatile underlying_t *)ptr = raw;   // FULL register write
	}
};

// ======================================================
// Const MMIO pointer (Can't change the pointer)
// ======================================================
/*
	This one isn't really needed. The other should compile to something as fast as this.
	But, if it doesn't, well you have this.
*/
template <typename T> struct mut_mmio_ptr
{
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "mmio_ptr only supports 1, 2, or 4 byte types");

	static constexpr bool T_is_uintX_t = is_same<T, uint8_t>::value || is_same<T, uint16_t>::value || is_same<T, uint32_t>::value;

  private:
	using underlying_t =
		typename conditional<sizeof(T) == 1, uint8_t, typename conditional<sizeof(T) == 2, uint16_t, uint32_t>::type>::type;

	volatile T *ptr; // pointer is const, can't be reassigned

  public:
	using value_type = T;

	constexpr explicit mut_mmio_ptr(volatile T *p) : ptr(p)
	{
	}

	T read() const
	{
		if constexpr (T_is_uintX_t)
		{
			return *ptr;
		}
		else
		{
			union
			{
				volatile underlying_t raw;
				T					  val;
			} u;
			u.raw = *(volatile underlying_t *)ptr;
			return u.val;
		}
	}

	void write(const T val) const
	{
		if constexpr (T_is_uintX_t)
		{
			*ptr = val;
		}
		else
		{

			union
			{
				T			 val;
				underlying_t raw;
			} u;
			u.val						  = val;
			*(volatile underlying_t *)ptr = u.raw;
		}
	}

#ifdef MEMCPY_VERSION
	T read_m() const
	{
		uint32_t raw = *(volatile uint32_t *)ptr; // FULL register read
		T		 val;
		__builtin_memcpy(&val, &raw, sizeof(T)); // copy into struct
		return val;
	}

	void write_m(const T &v) const
	{
		uint32_t raw = 0;
		__builtin_memcpy(&raw, &v, sizeof(T)); // copy struct → raw
		*(volatile uint32_t *)ptr = raw;	   // FULL register write
	}

	// no write() method — read-only
#endif
};

} // namespace std

void test_special_pointers();
