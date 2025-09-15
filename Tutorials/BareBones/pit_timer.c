#include <stdint.h>

#define PIT_CH0 0x40
#define PIT_CMD 0x43

static inline void outb(uint16_t port, uint8_t val) {
	__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void pit_init(uint16_t divisor) {
	outb(PIT_CMD, 0x36);                  // Channel 0, mode 3, lobyte/hibyte
	outb(PIT_CH0, divisor & 0xFF);        // Low byte
	outb(PIT_CH0, (divisor >> 8) & 0xFF); // High byte
}
