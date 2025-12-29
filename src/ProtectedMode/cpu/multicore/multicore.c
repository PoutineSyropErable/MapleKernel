#include "apic.h"
#include "multicore.h"

// Bit layout:
// bit 31: locked flag
// bits 16-30: recursion count (max 32767)
// bits 0-15: owner CPU ID (max 65535)
#define LOCK_FLAG (1U << 31)
#define RECURSION_SHIFT 16
#define RECURSION_MASK (0x7FFFU << RECURSION_SHIFT)
#define OWNER_MASK 0xFFFFU

bool reentrant_trylock(reentrant_lock_t *lock)
{
	uint32_t cpu_id = apic_get_core_id() & OWNER_MASK; // &owner mask unneeded
	uint32_t expected, desired;

	expected = atomic_load_explicit(&lock->state, memory_order_relaxed);

	do
	{
		uint32_t is_locked = expected & LOCK_FLAG;
		uint32_t owner	   = expected & OWNER_MASK;
		uint32_t recursion = (expected & RECURSION_MASK) >> RECURSION_SHIFT;

		if (is_locked && owner == cpu_id)
		{
			// We already own it
			if (recursion == 0x7FFF)
				return false;							  // Overflow
			desired = expected + (1U << RECURSION_SHIFT); // Increment recursion
		}
		else if (!is_locked)
		{
			// Acquire fresh
			desired = LOCK_FLAG | cpu_id | (1U << RECURSION_SHIFT);
		}
		else
		{
			// Locked by someone else
			return false;
		}
	} while (!atomic_compare_exchange_weak_explicit(&lock->state, &expected, desired, memory_order_acquire, memory_order_relaxed));

	return true;
}

void reentrant_lock(reentrant_lock_t *lock)
{
	uint32_t cpu_id = apic_get_core_id() & OWNER_MASK;

	// Fast path: might already own it
	uint32_t current = atomic_load_explicit(&lock->state, memory_order_relaxed);
	if ((current & (LOCK_FLAG | OWNER_MASK)) == (LOCK_FLAG | cpu_id))
	{
		// Might own it - try to increment recursion
		uint32_t expected = current;
		do
		{
			uint32_t recursion = (expected & RECURSION_MASK) >> RECURSION_SHIFT;
			if (recursion == 0x7FFF)
				break;

			uint32_t desired = expected + (1U << RECURSION_SHIFT);
			if (atomic_compare_exchange_weak_explicit(&lock->state, &expected, desired, memory_order_acquire, memory_order_relaxed))
			{
				return;
			}
		} while ((expected & (LOCK_FLAG | OWNER_MASK)) == (LOCK_FLAG | cpu_id));
	}

	// Need to acquire from scratch
	while (!reentrant_trylock(lock))
	{
		// Read-only spinning
		while (atomic_load_explicit(&lock->state, memory_order_relaxed) & LOCK_FLAG)
		{
			__asm__ volatile("pause");
		}
	}
}

void reentrant_unlock(reentrant_lock_t *lock)
{
	uint32_t cpu_id = apic_get_core_id() & OWNER_MASK;
	uint32_t expected, desired;

	expected = atomic_load_explicit(&lock->state, memory_order_relaxed);

	do
	{
		uint32_t is_locked = expected & LOCK_FLAG;
		uint32_t owner	   = expected & OWNER_MASK;
		uint32_t recursion = (expected & RECURSION_MASK) >> RECURSION_SHIFT;

		// Sanity check
		if (!is_locked || owner != cpu_id)
		{
			// ERROR: Trying to unlock lock we don't own
			return;
		}

		if (recursion > 1)
		{
			// Still recursive - decrement count
			desired = expected - (1U << RECURSION_SHIFT);
		}
		else
		{
			// Last unlock - release completely
			desired = 0;
		}
	} while (!atomic_compare_exchange_strong_explicit(&lock->state, &expected, desired, memory_order_release, memory_order_relaxed));
}
