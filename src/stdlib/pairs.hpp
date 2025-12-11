#pragma once

template <typename T1, typename T2> struct Pair
{
    T1 first;
    T2 second;

    // Default constructor
    constexpr Pair() : first{}, second{}
    {
    }

    // Constructor from values
    constexpr Pair(const T1 &a, const T2 &b) : first(a), second(b)
    {
    }

    // Copy constructor
    constexpr Pair(const Pair &other) : first(other.first), second(other.second)
    {
    }

    // Assignment
    constexpr Pair &operator=(const Pair &other)
    {
	first  = other.first;
	second = other.second;
	return *this;
    }

    // Swap utility (optional)
    constexpr void swap(Pair &other)
    {
	T1 tmp1      = first;
	T2 tmp2      = second;
	first        = other.first;
	second       = other.second;
	other.first  = tmp1;
	other.second = tmp2;
    }
};

template <typename T, typename E> struct ErrorPair
{
    T value;
    E error;

    // Default constructor
    constexpr ErrorPair() : value{}, error{}
    {
    }

    // Constructor from value and error
    constexpr ErrorPair(const T &v, const E &e) : value(v), error(e)
    {
    }

    // Copy constructor
    constexpr ErrorPair(const ErrorPair &other) : value(other.value), error(other.error)
    {
    }

    // Assignment
    constexpr ErrorPair &operator=(const ErrorPair &other)
    {
	value = other.value;
	error = other.error;
	return *this;
    }

    // Swap utility (optional)
    constexpr void swap(ErrorPair &other)
    {
	T tmpV      = value;
	E tmpE      = error;
	value       = other.value;
	error       = other.error;
	other.value = tmpV;
	other.error = tmpE;
    }
};
