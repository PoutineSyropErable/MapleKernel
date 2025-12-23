#pragma once
#include "assert.h"
#include "std.hpp"
#include <stdint.h>

namespace std
{

// Wrapper for a pointer that can only be set once
template <typename T> struct set_once_ptr
{
  private:
	T *ptr = nullptr;

  public:
	void set(T *p)
	{
#ifdef DEBUG
		assert(ptr == nullptr, "Pointer already set!");
#endif
		ptr = p;
	}

	T *get() const
	{
#ifdef DEBUG
		assert(ptr != nullptr, "Pointer not initialized!");
#endif
		return ptr;
	}

	// Convenient operator overloads to use like a pointer
	T &operator*() const
	{
		return *get();
	}
	T *operator->() const
	{
		return get();
	}
};

template <typename T> struct mmio_pointer
{
	volatile T *ptr;

	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "mmio_pointer only supports types of size 1, 2, or 4 bytes");

	// Determine underlying integer type for type-punning
	using underlying_t =
		typename conditional<sizeof(T) == 1, uint8_t, typename conditional<sizeof(T) == 2, uint16_t, uint32_t>::type>::type;

	constexpr mmio_pointer(volatile T *p = nullptr) : ptr(p)
	{
	}

	T read() const
	{
		// Force a stack allocation. ARRRGH
		union
		{
			volatile underlying_t raw;
			T					  val;
		} u;
		u.raw = *(volatile underlying_t *)ptr; // safe volatile read

		return u.val;
	}

	void write(T val) const
	{
		union
		{
			T			 val;
			underlying_t raw;
		} u;
		u.val						  = val;
		*(volatile underlying_t *)ptr = u.raw;
	}
};

// Simple MMIO pointer abstraction
template <typename T> struct mmio_ptr
{
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "mmio_ptr only supports 1, 2, or 4 byte types");

  private:
	using underlying_t =
		typename conditional<sizeof(T) == 1, uint8_t, typename conditional<sizeof(T) == 2, uint16_t, uint32_t>::type>::type;
	volatile T *const ptr; // pointer is immutable after construction

  public:
	using value_type = T; // <-- This is needed for wrappers like set_once
	// Construct with the raw pointer
	constexpr explicit mmio_ptr(volatile T *p) : ptr(p)
	{
	}

	// Read value (for read-capable pointers)
	T read() const
	{
		union
		{
			volatile underlying_t raw;
			T					  val;
		} u;
		u.raw = *(volatile underlying_t *)ptr; // safe volatile read
		return u.val;
	}

	// Write value (for write-capable pointers)
	void write(const T val) const
	{
		union
		{
			T			 val;
			underlying_t raw;
		} u;

		u.val						  = val;
		*(volatile underlying_t *)ptr = u.raw;
	}
};

// ======================================================
// Set-once wrappers enforcing access restrictions
// ======================================================

// Read/Write set-once pointer
template <typename MMIO> struct set_once
{
  private:
	const MMIO *mmio_ptr = nullptr; // pointer to a const MMIO

  public:
	using value_type = typename MMIO::value_type;

	constexpr set_once() = default;

	// Set the MMIO pointer exactly once
	void set(const MMIO *p)
	{
#ifdef DEBUG
		assert(mmio_ptr == nullptr, "Pointer already initialized!");
#endif
		mmio_ptr = p;
	}

	// Set from raw volatile pointer
	void set(volatile value_type *p)
	{
#ifdef DEBUG
		assert(mmio_ptr == nullptr, "Pointer already initialized!");
#endif
		mmio_ptr = MMIO(p); // construct the wrapper
	}

	// Read the register at the MMIO
	value_type read() const
	{
#ifdef DEBUG
		assert(mmio_ptr != nullptr, "Pointer not initialized!");
#endif
		return mmio_ptr->read();
	}

	// Write to the register at the MMIO
	void write(const value_type &v) const
	{
#ifdef DEBUG
		assert(mmio_ptr != nullptr, "Pointer not initialized!");
#endif
		mmio_ptr->write(v);
	}
};

// Read Only set-once pointer
template <typename MMIO> struct set_once_ro
{
  private:
	const MMIO *mmio_ptr = nullptr; // pointer to a const MMIO

  public:
	using value_type = typename MMIO::value_type;

	constexpr set_once_ro() = default;

	// Set the MMIO pointer exactly once
	void set(const MMIO *p)
	{
		if (mmio_ptr == nullptr)
		{
			mmio_ptr = p;
		}
		else
		{
			// Optionally: error handling if someone tries to set again
		}
	}

	// Set from raw volatile pointer
	void set(volatile value_type *p)
	{
		mmio_ptr = MMIO(p); // construct the wrapper
	}

	// Read the register at the MMIO
	value_type read() const
	{
		return mmio_ptr->read();
	}
};

// write only Only set-once pointer
template <typename MMIO> struct set_once_wo
{
  private:
	const MMIO *mmio_ptr = nullptr; // pointer to a const MMIO

  public:
	using value_type = typename MMIO::value_type;

	constexpr set_once_wo() = default;

	// Set the MMIO pointer exactly once
	void set(const MMIO *p)
	{
		if (mmio_ptr == nullptr)
		{
			mmio_ptr = p;
		}
		else
		{
			// Optionally: error handling if someone tries to set again
		}
	}

	// Read the register at the MMIO
	value_type write() const
	{
		return mmio_ptr->write();
	}
};

} // namespace std

void test_special_pointers();
