#pragma once
#include <stdint.h>
// This is for kernel

#ifdef __cplusplus
extern "C"
{
#endif
	// Clear mask is to ENABLE THE IRQ
	void IRQ_clear_mask(uint8_t IRQline);
	// Set mask is to ENABLE THE IRQ
	void IRQ_set_mask(uint8_t IRQline);
	void initialize_irqs();

	// This one is for hardware
	void PIC_sendEOI(uint8_t irq);

#ifdef __cplusplus
}
#endif
