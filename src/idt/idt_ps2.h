#pragma once
#include "idt.h"
#include "ps2.h"
enum idt_init_type {
	ITT_one_keyboard_one_mouse,
	ITT_two_keyboard,
	ITT_two_mouse,
	ITT_one_keyboard,
	ITT_one_mouse,
	ITT_no_ps2_device,
};

struct idt_fields_1k_1m {
	uint8_t keyboard_port;
	uint8_t mouse_port;
};

struct idt_fields_2k {
	uint8_t keyboard1_port;
	uint8_t keyboard2_port;
};

struct idt_fields_2m {
	uint8_t mouse1_port;
	uint8_t mouse2_port;
};

struct idt_fields_1k {
	uint8_t keyboard_type;
};

struct idt_fields_1m {
	uint8_t mouse_type;
};

struct idt_fields_none {
	[[gnu::unused]] uint8_t empty;
};

union idt_temp {
	struct idt_fields_1k_1m mk;
	struct idt_fields_2k kk;
	struct idt_fields_2m mm;
	struct idt_fields_1k k;
	struct idt_fields_1m m;
	struct idt_fields_none n;
};

struct idt_init_fields {
	enum idt_init_type type;
	union idt_temp value;
};

void idt_init(struct idt_init_fields fields);
