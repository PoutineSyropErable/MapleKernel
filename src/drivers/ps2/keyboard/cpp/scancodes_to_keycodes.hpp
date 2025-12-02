#pragma once
#include "keycodes.hpp"

namespace keycodes
{
constexpr size_t SCANCODE_COUNT          = 128; // 8 down positions * 32 right positions
constexpr size_t EXTENDED_SCANCODE_COUNT = 128; // 8 down positions * 32 right positions

extern KeyCode ScanToKeyCode[SCANCODE_COUNT];
extern KeyCode ExtendedScanToKeyCode[SCANCODE_COUNT];

void init_scancodes_to_keycodes();
} // namespace keycodes
