#pragma once
#include "ps2_keyboard_handler_public.h"

void setup_keyboard_extra();

inline bool is_shift_held();

extern bool is_ctrl_held();

extern inline bool is_alt_held();

extern inline bool is_altgr_held();

extern inline bool is_capslock_on();

extern inline bool is_caps();
