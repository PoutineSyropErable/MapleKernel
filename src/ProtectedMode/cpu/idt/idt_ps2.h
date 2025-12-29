#pragma once
#include "idt.h" // only needed for the 256 value...
#include "ps2_controller_public.h"
#include "stdint.h"
enum idt_init_type
{
    ITT_one_keyboard_one_mouse = 0,
    ITT_two_keyboard,
    ITT_two_mouse,
    ITT_one_keyboard,
    ITT_one_mouse,
    ITT_no_ps2_device,
};

// no mouse

struct idt_fields_keyboard_mouse
{
    uint8_t              keyboard_port;
    uint8_t              mouse_port;
    enum ps2_device_type keyboard_type;
    enum ps2_device_type mouse_type;
};

struct idt_fields_2_keyboard
{
    uint8_t              keyboard1_port; // always = 1
    uint8_t              keyboard2_port; // always = 2
    enum ps2_device_type keyboard1_type;
    enum ps2_device_type keyboard2_type;
};

struct idt_fields_2_mouse
{
    uint8_t              mouse1_port; // always = 1
    uint8_t              mouse2_port; // always = 2
    enum ps2_device_type mouse1_type;
    enum ps2_device_type mouse2_type;
};

struct idt_fields_1_keyboard
{
    uint8_t keyboard_type;
};

struct idt_fields_1_mouse
{
    uint8_t mouse_type;
};

struct idt_fields_none
{
    [[gnu::unused]] uint8_t empty;
};

union idt_temp
{
    struct idt_fields_keyboard_mouse info_keyboard_and_mouse;
    struct idt_fields_2_keyboard     info_2_keyboard;
    struct idt_fields_2_mouse        info_2_mouse;
    struct idt_fields_1_keyboard     info_1_keyboard;
    struct idt_fields_1_mouse        info_1_mouse;
    struct idt_fields_none           info_none;
};

struct idt_init_ps2_fields
{
    enum idt_init_type type;
    union idt_temp     value;
};

void idt_init_ps2(struct idt_init_ps2_fields args);

extern bool vectors[IDT_MAX_VECTOR_COUNT];
