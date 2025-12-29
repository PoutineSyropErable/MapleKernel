#pragma once
#include "pic.h"
#include <stdint.h>

static const uint8_t PS2_PORT1_INTERUPT_VECTOR = (START_OF_USER_ALLOCATABLE_INTERRUPT + PS2_PORT1_IRQ);
static const uint8_t PS2_PORT2_INTERUPT_VECTOR = (START_OF_USER_ALLOCATABLE_INTERRUPT + PS2_PORT2_IRQ);

static const uint8_t interrupt_vector_of_port[3] = {
    [0] = bad_port,
    [1] = PS2_PORT1_INTERUPT_VECTOR,
    [2] = PS2_PORT2_INTERUPT_VECTOR,
};

static uint8_t irq_of_ps2_port[3] = {
    [0] = bad_irq,
    [1] = PS2_PORT1_IRQ,
    [2] = PS2_PORT2_IRQ,
};
