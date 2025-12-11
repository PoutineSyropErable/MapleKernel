#pragma once
#include <stdint.h>

// This is for kernel
void IRQ_clear_mask(uint8_t IRQline);
void IRQ_set_mask(uint8_t IRQline);
void initialize_irqs();

// This one is for hardware
void PIC_sendEOI(uint8_t irq);
