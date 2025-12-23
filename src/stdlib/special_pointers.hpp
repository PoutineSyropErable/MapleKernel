#pragma once
#include "assert.h"
#include "std.hpp"
#include <stdint.h>

namespace std
{

// #define DEBUG

// Wrapper for a pointer that can only be set once
template <typename T> struct set_once_primitive_ptr
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
#ifdef DEBUG
		assert(ptr != nullptr, "Can't read a null mmio_ptr!\n");
#endif

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
#ifdef DEBUG
		assert(ptr != nullptr, "Can't read a null mmio_ptr!\n");
#endif
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
template <typename T, bool IsConst = true> struct mmio_ptr
{
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "mmio_ptr only supports 1, 2, or 4 byte types");

  private:
	using underlying_t =
		typename conditional<sizeof(T) == 1, uint8_t, typename conditional<sizeof(T) == 2, uint16_t, uint32_t>::type>::type;

	// Choose pointer type based on IsConst
	using ptr_type = typename conditional<IsConst,
		volatile T *const,	 // if wrapper is const
		volatile T *>::type; // otherwise

	ptr_type ptr; // underlying pointer

  public:
	using value_type = T; // <-- This is needed for wrappers like set_once
	// Construct with the raw pointer
	constexpr explicit mmio_ptr(volatile T *p) : ptr(p)
	{
	}

	// Default constructor → pointer is null
	constexpr mmio_ptr() : ptr(nullptr)
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
// ======================================================
// Set-once wrapper (read/write)
// ======================================================
template <typename MMIO> struct set_once
{
  private:
	MMIO mmio; // stored MMIO wrapper
#ifdef DEBUG
	bool is_set = false; // ensures single initialization
#endif

  public:
	using value_type = typename MMIO::value_type;

	constexpr set_once() = default;

	// Set from an mmio_ptr; can only be called once
	void set(const MMIO &m)
	{
#ifdef DEBUG

		assert(!is_set, "Ptr is already set\n");
#endif

		mmio = m;
#ifdef DEBUG
		is_set = true;
#endif
	}

	value_type read() const
	{
#ifdef DEBUG
		assert(is_set, "ptr must be set to read it\n");
#endif
		return mmio.read();
	}

	void write(const value_type &v) const
	{
#ifdef DEBUG
		assert(is_set, "ptr must be set to write it\n");
#endif
		mmio.write(v);
	}
};

// ======================================================
// Read-only set-once pointer
// ======================================================
template <typename MMIO> struct set_once_ro
{
  private:
	MMIO mmio; // stored MMIO wrapper
#ifdef DEBUG
	bool is_set = false;
#endif

  public:
	using value_type = typename MMIO::value_type;

	constexpr set_once_ro() = default;

	// Set the MMIO pointer exactly once
	void set(const MMIO &m)
	{
#ifdef DEBUG
		assert(!is_set, "Pointer already initialized!\n");
#endif
		mmio = m;
#ifdef DEBUG
		is_set = true;
#endif
	}

	value_type read() const
	{
#ifdef DEBUG
		assert(is_set, "Pointer not initialized!\n");
#endif
		return mmio.read();
	}

	// Delete write to enforce read-only access
	void write(const value_type &) const = delete;
};

// ======================================================
// Write-only set-once pointer
// ======================================================
template <typename MMIO> struct set_once_wo
{
  private:
	MMIO mmio; // stored MMIO wrapper
#ifdef DEBUG
	bool is_set = false;
#endif

  public:
	using value_type = typename MMIO::value_type;

	constexpr set_once_wo() = default;

	// Set the MMIO pointer exactly once
	void set(const MMIO &m)
	{
#ifdef DEBUG
		assert(!is_set, "Pointer already initialized!\n");
#endif
		mmio = m;
#ifdef DEBUG
		is_set = true;
#endif
	}

	void write(const value_type &v) const
	{
#ifdef DEBUG
		assert(is_set, "Pointer not initialized!\n");
#endif
		mmio.write(v);
	}

	// Delete read to enforce write-only access
	value_type read() const = delete;
};

} // namespace std

void test_special_pointers();
