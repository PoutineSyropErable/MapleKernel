#pragma once
#include "idt.h"
#include "idt_ps2.h"
#include "idt_test.h"

void idt_init(struct idt_init_ps2_fields fields);
