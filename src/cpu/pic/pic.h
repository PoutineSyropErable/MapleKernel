#pragma once
#include <stdint.h>

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20 /* End-of-interrupt command code */

// TODO: Double check them. (This goes with the other todo of not using magic numbers)
// To what index are Interrupt reserved in PM and LM?
#define PS2_PORT1_IRQ 1
#define PS2_PORT2_BRIDGE_IRQ 2
#define PS2_PORT2_IRQ 12
#define START_OF_USER_ALLOCATABLE_INTERRUPT 32
#define NUMBER_OF_IRQ_PER_PIC 8

// These are changeable. It doesn't have to be this formula

static const uint8_t PIC_1_OFFSET = START_OF_USER_ALLOCATABLE_INTERRUPT;
static const uint8_t PIC_2_OFFSET = START_OF_USER_ALLOCATABLE_INTERRUPT + NUMBER_OF_IRQ_PER_PIC;

static const uint8_t bad_port = 255;
static const uint8_t bad_irq  = 255;

void PIC_remap(int offset1, int offset2);
