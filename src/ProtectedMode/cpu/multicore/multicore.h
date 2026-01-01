#pragma once
#include "atomic.h"
#include "core_count.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	extern volatile bool core_has_booted[MAX_CORE_COUNT];			 // [i = reciever][ j = sender]
	extern volatile bool master_tells_core_to_start[MAX_CORE_COUNT]; // [i = reciever][ j = sender]
																	 // extern volatile void (*core_mains[8])();
	extern void (*volatile core_mains[MAX_CORE_COUNT])();
	extern volatile uint8_t last_interrupt_received[MAX_CORE_COUNT][MAX_CORE_COUNT]; // [i = reciever][ j = sender]
	extern uint8_t			runtime_core_count;

#define SPURIOUS_INTERRUPT_VECTOR 56 // 32 + 24 = reserved interrupt + 24 irqs.

#define NO_INTERRUPT 255

	extern void core_bootstrap();
	extern void application_core_main();

	typedef struct
	{
		uint32_t state;

	} reentrant_lock_t;

	void reentrant_lock(reentrant_lock_t *lock);
	void reentrant_unlock(reentrant_lock_t *lock);
	bool reentrant_trylock(reentrant_lock_t *lock);

	static inline uint32_t irq_save(void)
	{
		uint32_t flags;
		__asm__ volatile("pushf\n\t"
						 "pop %0\n\t"
						 "cli"
			: "=r"(flags)
			:
			: "memory");
		return flags;
	}

	static inline void irq_restore(uint32_t flags)
	{
		__asm__ volatile("push %0\n\t"
						 "popf"
			:
			: "r"(flags)
			: "memory", "cc");
	}

#ifdef __cplusplus
}
#endif
