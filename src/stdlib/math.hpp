#pragma once

template <typename T, typename... Ts> constexpr T min(T a, Ts... rest)
{
	if constexpr (sizeof...(rest) == 0)
		return a;
	else
	{
		T next_min = min(rest...);
		return (a < next_min) ? a : next_min;
	}
}

template <typename T, typename... Ts> constexpr T max(T a, Ts... rest)
{
	if constexpr (sizeof...(rest) == 0)
		return a;
	else
	{
		T next_max = max(rest...);
		return (a > next_max) ? a : next_max;
	}
}

// Absolute value
template <typename T> constexpr T abs(T x) noexcept
{
	return (x < static_cast<T>(0)) ? -x : x;
}
