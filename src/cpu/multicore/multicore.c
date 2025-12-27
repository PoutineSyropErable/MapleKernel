#include "multicore.h"

static inline bool try_lock(lock_t *lock)
{
	uint32_t eax = 1;
	__asm__ volatile("lock xchg %0, %1" : "+r"(eax), "+m"(lock->is_locked) : : "memory");
	// if is_locked = 1, then eax = 1.
	// if is_locked = 0, then eax =0 at the end
	return eax == 0;
}

void spinlock(lock_t *lock)
{
	while (!try_lock(lock))
	{
		__asm__ volatile("pause");
	}
}

void unlock(lock_t *lock)
{
	__atomic_store_n(&lock->is_locked, 0, __ATOMIC_RELEASE);
}
