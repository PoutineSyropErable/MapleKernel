#pragma once
#include "pic.h"
#include <stdint.h>

static const uint8_t PIT_INTERUPT_VECTOR = (START_OF_USER_ALLOCATABLE_INTERRUPT + PIT_IRQ);
