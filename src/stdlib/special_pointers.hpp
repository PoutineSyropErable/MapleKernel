#pragma once
#include "assert.h"
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

// Freestanding conditional template
template <bool B, typename T, typename F> struct conditional
{
	using type = T;
};

template <typename T, typename F> struct conditional<false, T, F>
{
	using type = F;
};

// Freestanding remove_reference
template <typename T> struct remove_reference
{
	using type = T;
};
template <typename T> struct remove_reference<T &>
{
	using type = T;
};
template <typename T> struct remove_reference<T &&>
{
	using type = T;
};

template <typename T> constexpr typename remove_reference<T>::type &&move(T &&t) noexcept
{
	return static_cast<typename remove_reference<T>::type &&>(t);
}

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

} // namespace std

void test_special_pointers();
