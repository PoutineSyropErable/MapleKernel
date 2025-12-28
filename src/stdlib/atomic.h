// atomic.h
#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	// Memory order aliases (matching C11)
	typedef enum
	{
		memory_order_relaxed = __ATOMIC_RELAXED,
		memory_order_consume = __ATOMIC_CONSUME,
		memory_order_acquire = __ATOMIC_ACQUIRE,
		memory_order_release = __ATOMIC_RELEASE,
		memory_order_acq_rel = __ATOMIC_ACQ_REL,
		memory_order_seq_cst = __ATOMIC_SEQ_CST
	} memory_order;

	// Compiler barrier (no CPU fence)
	static inline void compiler_barrier(void)
	{
		__asm__ volatile("" ::: "memory");
	}

	// CPU memory barriers (use sparingly!)
	static inline void mb_full(void)
	{
		__asm__ volatile("mfence" ::: "memory");
	}

	static inline void mb_load(void)
	{
		__asm__ volatile("lfence" ::: "memory");
	}

	static inline void mb_store(void)
	{
		__asm__ volatile("sfence" ::: "memory");
	}

	// Generic atomic load with specified ordering
	static inline uint32_t atomic_load_explicit(const volatile uint32_t *ptr, memory_order order)
	{
		return __atomic_load_n(ptr, order);
	}

	static inline uint32_t atomic_load(const volatile uint32_t *ptr)
	{
		return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
	}

	// Generic atomic store
	static inline void atomic_store_explicit(volatile uint32_t *ptr, uint32_t val, memory_order order)
	{
		__atomic_store_n(ptr, val, order);
	}

	static inline void atomic_store(volatile uint32_t *ptr, uint32_t val)
	{
		__atomic_store_n(ptr, val, __ATOMIC_RELEASE);
	}

	// Exchange
	static inline uint32_t atomic_exchange_explicit(volatile uint32_t *ptr, uint32_t val, memory_order order)
	{
		return __atomic_exchange_n(ptr, val, order);
	}

	static inline uint32_t atomic_exchange(volatile uint32_t *ptr, uint32_t val)
	{
		return __atomic_exchange_n(ptr, val, __ATOMIC_ACQ_REL);
	}

	// Compare-and-swap with explicit ordering
	static inline bool atomic_compare_exchange_weak_explicit(
		volatile uint32_t *ptr, uint32_t *expected, uint32_t desired, memory_order success, memory_order failure)
	{
		return __atomic_compare_exchange_n(ptr, expected, desired,
			true, // true = weak
			success, failure);
	}

	static inline bool atomic_compare_exchange_strong_explicit(
		volatile uint32_t *ptr, uint32_t *expected, uint32_t desired, memory_order success, memory_order failure)
	{
		return __atomic_compare_exchange_n(ptr, expected, desired,
			false, // false = strong
			success, failure);
	}

	// Convenience CAS (good for locks)
	static inline bool atomic_cas(volatile uint32_t *ptr, uint32_t *expected, uint32_t desired)
	{
		// Strong CAS, ACQ_REL on success, ACQUIRE on failure
		return __atomic_compare_exchange_n(ptr, expected, desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
	}

	// Fetch-and-add
	static inline uint32_t atomic_fetch_add_explicit(volatile uint32_t *ptr, uint32_t val, memory_order order)
	{
		return __atomic_fetch_add(ptr, val, order);
	}

	static inline uint32_t atomic_fetch_add(volatile uint32_t *ptr, uint32_t val)
	{
		return __atomic_fetch_add(ptr, val, __ATOMIC_ACQ_REL);
	}

	// Fetch-and-sub
	static inline uint32_t atomic_fetch_sub_explicit(volatile uint32_t *ptr, uint32_t val, memory_order order)
	{
		return __atomic_fetch_sub(ptr, val, order);
	}

	static inline uint32_t atomic_fetch_sub(volatile uint32_t *ptr, uint32_t val)
	{
		return __atomic_fetch_sub(ptr, val, __ATOMIC_ACQ_REL);
	}

	// ====================

	// For spinlocks specifically - optimized versions
	static inline bool spin_trylock(volatile uint32_t *lock)
	{
		uint32_t expected = 0;
		// Weak CAS is better for spinlocks - can fail spuriously
		return __atomic_compare_exchange_n(lock, &expected, 1,
			true, // weak
			__ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
	}

	static inline void simple_spin_lock(volatile uint32_t *lock)
	{
		// Phase 1: Read-only spin (relaxed - just checking value)
		while (__atomic_load_n(lock, __ATOMIC_RELAXED) != 0)
		{
			__asm__ volatile("pause");
		}

		// Phase 2: Try to acquire with CAS
		uint32_t expected = 0;
		while (!__atomic_compare_exchange_n(lock, &expected, 1,
			true, // weak CAS
			__ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
		{
			// Failed - back to read-only spin
			expected = 0;
			while (__atomic_load_n(lock, __ATOMIC_RELAXED) != 0)
			{
				__asm__ volatile("pause");
			}
		}
	}

	static inline void simple_spin_unlock(volatile uint32_t *lock)
	{
		atomic_store(lock, 0); // With RELEASE semantics
	}

#ifdef __cplusplus
}
#endif

#endif // ATOMIC_H
