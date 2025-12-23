#pragma once
#include "assert.h"

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

} // namespace std
