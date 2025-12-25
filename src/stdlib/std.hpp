#pragma once
#include <stdint.h>

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

// ===================================== To and from uints

template <typename T> constexpr T from_uint32(uint32_t raw)
{
	union
	{
		uint32_t r;
		T		 val;
	} u;
	u.r = raw;
	return u.val;
}

template <typename T> constexpr uint32_t to_uint32(T val)
{
	union
	{
		uint32_t r;
		T		 val;
	} u;
	u.val = val;
	return u.r;
}

// ================ Is same ==========

// Default case: T and U are different
template <typename T, typename U> struct is_same
{
	static constexpr bool value = false;
};

// Specialization: T and U are the same
template <typename T> struct is_same<T, T>
{
	static constexpr bool value = true;
};

// ====== Is type uint =============
//
// Helper to find if it's a primitive type
//

template <typename T> struct is_type_uint
{
	static constexpr bool value = false;
};

template <> struct is_type_uint<uint8_t>
{
	static constexpr bool value = true;
};
template <> struct is_type_uint<uint16_t>
{
	static constexpr bool value = true;
};
template <> struct is_type_uint<uint32_t>
{
	static constexpr bool value = true;
};
