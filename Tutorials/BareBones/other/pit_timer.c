#include "pit_timer.h"
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
	__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void pit_init(uint16_t divisor) {
	outb(PIT_CMD, 0x36);                  // Channel 0, mode 3, lobyte/hibyte
	outb(PIT_CH0, divisor & 0xFF);        // Low byte
	outb(PIT_CH0, (divisor >> 8) & 0xFF); // High byte
}

void wait(float seconds) {

	const unsigned long loops_per_sec = 150000000UL; // tuned for ~1s per unit

	// total loops = seconds * loops_per_sec
	unsigned long total_loops = (unsigned long)(seconds * loops_per_sec);

	for (unsigned long i = 0; i < total_loops; i++) {
		__asm__ volatile("nop");
	}
}
