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
