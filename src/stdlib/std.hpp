#pragma once

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
