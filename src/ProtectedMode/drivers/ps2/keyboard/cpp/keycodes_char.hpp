#pragma once
#include "keycodes.hpp"

namespace keycodes
{

// Since keys are packed as (down << 5) | right, we have 8*32 = 256 possible values
constexpr size_t KEYCODE_COUNT = 256; // 8 down positions * 32 right positions

struct KeyCharInfo
{
    char normal;       // No modifiers
    char shift;        // With Shift
    char caps;         // With CapsLock only (for letters)
    char altgr;        // with altgr
    bool is_letter;    // Is this key a letter (A-Z)
    bool is_printable; // Does it produce any character?
    bool is_altgr_able;
};

// Statically initialized array - keycode is the index
extern KeyCharInfo KEY_CHAR_TABLE[KEYCODE_COUNT];

// Then fill specific positions
// Numbers row (down = 1)

// Direct array access - O(1) lookup!
constexpr const KeyCharInfo &get_char_info(Key key)
{
    return KEY_CHAR_TABLE[static_cast<uint8_t>(key)];
}

// Is printable character
constexpr bool is_printable(Key key)
{
    return get_char_info(key).is_printable;
}

// Is letter (A-Z)
constexpr bool is_letter(Key key)
{
    return get_char_info(key).is_letter;
}

constexpr bool is_altgr_able(Key key)
{
    return get_char_info(key).is_altgr_able;
}

// Convert key to character based on modifiers
constexpr char key_to_char(Key key, const KeyModifierState &mods)
{
    const auto &info = KEY_CHAR_TABLE[static_cast<uint8_t>(key)];
    if (!info.is_printable)
        return 0;

    if (!info.is_altgr_able && mods.altgr)
    {
        return 0;
    }

    if (mods.left_ctrl || mods.right_ctrl || mods.left_meta || mods.right_meta || mods.alt)
    {
        return 0;
    }

    bool shift = mods.left_shift || mods.right_shift;
    bool caps  = mods.capslock;
    bool altgr = mods.altgr;

    if (info.is_letter)
    {
        // Letters have special CapsLock behavior
        if (shift && caps)
        {
            return info.normal; // Shift + CapsLock = lowercase
        }
        else if (caps)
        {
            return info.caps; // CapsLock only = uppercase
        }
        else if (shift)
        {
            return info.shift; // Shift only = uppercase
        }
        else
        {
            return info.normal; // No modifiers = lowercase
        }
    }
    else
    {
        // Non-letters: CapsLock doesn't affect them
        if (altgr)
        {
            return info.altgr;
        }
        else if (shift)
        {
            return info.shift; // Shift = alternate symbol
        }
        else
        {
            return info.normal; // No shift = normal symbol
        }
    }
}

} // namespace keycodes
