#pragma once
#include "assert.h"
#include "std.hpp"
#include <stdint.h>

// REQUIRES: -fno-strict-aliasing

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

template <typename T> struct primitive_mmio_ptr
{

  private:
	volatile T *ptr; // mutable pointer. But never

  public:
	using value_type = T;

	// construct from raw pointer
	constexpr primitive_mmio_ptr(volatile T *p = nullptr) : ptr(p)
	{
	}

	T read() const
	{
		return *ptr;
	}

	void write(const T val) const
	{
		*ptr = val;
	}
};

// ======================================================================
// Modifiable (but private) MMIO pointer (This is the one you should use)
// ======================================================================
template <typename T> struct mmio_ptr
{
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "mmio_ptr only supports 1, 2, or 4 byte types");

  private:
	using underlying_t =
		typename conditional<sizeof(T) == 1, uint8_t, typename conditional<sizeof(T) == 2, uint16_t, uint32_t>::type>::type;

	volatile T *ptr; // mutable pointer. But never

  public:
	using value_type = T;

	// construct from raw pointer
	constexpr explicit mmio_ptr(volatile T *p) : ptr(p)
	{
	}

	// default constructor → pointer is null
	constexpr mmio_ptr() : ptr(nullptr)
	{
	}

	T read() const
	{
		union
		{
			volatile underlying_t raw;
			T					  val;
		} u;
		u.raw = *(volatile underlying_t *)ptr;
		return u.val;
	}

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

#ifdef MALOC_VERSION
	T read_m() const
	{
		uint32_t raw = *(volatile uint32_t *)ptr; // FULL register read
		T		 val;
		__builtin_memcpy(&val, &raw, sizeof(T)); // copy into struct
		return val;
	}

	void write_m(const T v) const
	{
		uint32_t raw = 0;
		__builtin_memcpy(&raw, &v, sizeof(T)); // copy struct → raw
		*(volatile uint32_t *)ptr = raw;	   // FULL register write
	}
#endif
};

// ======================================================
// Const MMIO pointer (Can't change the pointer)
// ======================================================
/*
	This one isn't really needed. The other should compile to something as fast as this.
	But, if it doesn't, well you have this.
*/
template <typename T> struct const_mmio_ptr
{
	static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "mmio_ptr only supports 1, 2, or 4 byte types");

  private:
	using underlying_t =
		typename conditional<sizeof(T) == 1, uint8_t, typename conditional<sizeof(T) == 2, uint16_t, uint32_t>::type>::type;

	volatile T *const ptr; // pointer is const, can't be reassigned

  public:
	using value_type = T;

	constexpr explicit const_mmio_ptr(volatile T *p) : ptr(p)
	{
	}

	T read() const
	{
		union
		{
			volatile underlying_t raw;
			T					  val;
		} u;
		u.raw = *(volatile underlying_t *)ptr;
		return u.val;
	}

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

#ifdef MALOC_VERSION
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

#ifdef MALOC_VERSION
	value_type read_m() const
	{
#	ifdef DEBUG
		assert(is_set, "ptr must be set to read it\n");
#	endif
		return mmio.read_m();
	}

	void write_m(const value_type &v) const
	{
#	ifdef DEBUG
		assert(is_set, "ptr must be set to write it\n");
#	endif
		mmio.write_m(v);
	}
#endif
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

#ifdef MALOC_VERSION
	value_type read_m() const
	{
#	ifdef DEBUG
		assert(is_set, "ptr must be set to read it\n");
#	endif
		return mmio.read_m();
	}
#endif
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

#ifdef MALOC_VERSION
	void write_m(const value_type &v) const
	{
#	ifdef DEBUG
		assert(is_set, "ptr must be set to write it\n");
#	endif
		mmio.write_m(v);
	}
#endif
};

} // namespace std

void test_special_pointers();
