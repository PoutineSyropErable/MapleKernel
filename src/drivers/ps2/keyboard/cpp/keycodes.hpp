#pragma once
#include "keycodes.h"

namespace keycodes
{

constexpr uint8_t pack_keycode_raw(const struct KeyCode keycode)
{
    return (keycode.down << 5) | (keycode.right & 0b11111);
    // Forced to use a bad and dumb trick
    // What if i change how keybind is done?
}

enum class Key : uint8_t
{
    // KEY_ESC = pack_keycode_simple(KeyCodes{.right = 0, .down = 0}),
    KEY_ESC        = pack_keycode_raw(KeyCode{.right = 0, .down = 0}),
    KEY_F1         = pack_keycode_raw(KeyCode{.right = 1, .down = 0}),
    KEY_F2         = pack_keycode_raw(KeyCode{.right = 2, .down = 0}),
    KEY_F3         = pack_keycode_raw(KeyCode{.right = 3, .down = 0}),
    KEY_F4         = pack_keycode_raw(KeyCode{.right = 4, .down = 0}),
    KEY_F5         = pack_keycode_raw(KeyCode{.right = 5, .down = 0}),
    KEY_F6         = pack_keycode_raw(KeyCode{.right = 6, .down = 0}),
    KEY_F7         = pack_keycode_raw(KeyCode{.right = 7, .down = 0}),
    KEY_F8         = pack_keycode_raw(KeyCode{.right = 8, .down = 0}),
    KEY_F9         = pack_keycode_raw(KeyCode{.right = 9, .down = 0}),
    KEY_F10        = pack_keycode_raw(KeyCode{.right = 10, .down = 0}),
    KEY_F11        = pack_keycode_raw(KeyCode{.right = 11, .down = 0}),
    KEY_F12        = pack_keycode_raw(KeyCode{.right = 12, .down = 0}),
    KEY_PRINTSCR   = pack_keycode_raw(KeyCode{.right = 13, .down = 0}),
    KEY_SCROLLLOCK = pack_keycode_raw(KeyCode{.right = 14, .down = 0}),
    KEY_PAUSE      = pack_keycode_raw(KeyCode{.right = 15, .down = 0}),

    // Row 1 - Number row
    KEY_BACKTICK    = pack_keycode_raw(KeyCode{.right = 0, .down = 1}),
    KEY_1           = pack_keycode_raw(KeyCode{.right = 1, .down = 1}),
    KEY_2           = pack_keycode_raw(KeyCode{.right = 2, .down = 1}),
    KEY_3           = pack_keycode_raw(KeyCode{.right = 3, .down = 1}),
    KEY_4           = pack_keycode_raw(KeyCode{.right = 4, .down = 1}),
    KEY_5           = pack_keycode_raw(KeyCode{.right = 5, .down = 1}),
    KEY_6           = pack_keycode_raw(KeyCode{.right = 6, .down = 1}),
    KEY_7           = pack_keycode_raw(KeyCode{.right = 7, .down = 1}),
    KEY_8           = pack_keycode_raw(KeyCode{.right = 8, .down = 1}),
    KEY_9           = pack_keycode_raw(KeyCode{.right = 9, .down = 1}),
    KEY_0           = pack_keycode_raw(KeyCode{.right = 10, .down = 1}),
    KEY_MINUS       = pack_keycode_raw(KeyCode{.right = 11, .down = 1}),
    KEY_EQUALS      = pack_keycode_raw(KeyCode{.right = 12, .down = 1}),
    KEY_BACKSPACE   = pack_keycode_raw(KeyCode{.right = 13, .down = 1}),
    KEY_INSERT      = pack_keycode_raw(KeyCode{.right = 14, .down = 1}),
    KEY_HOME        = pack_keycode_raw(KeyCode{.right = 15, .down = 1}),
    KEY_PAGEUP      = pack_keycode_raw(KeyCode{.right = 16, .down = 1}),
    KEY_NUMLOCK     = pack_keycode_raw(KeyCode{.right = 17, .down = 1}),
    KEY_KP_DIVIDE   = pack_keycode_raw(KeyCode{.right = 18, .down = 1}),
    KEY_KP_MULTIPLY = pack_keycode_raw(KeyCode{.right = 19, .down = 1}),
    KEY_KP_MINUS    = pack_keycode_raw(KeyCode{.right = 20, .down = 1}),

    // Row 2 - QWERTY row
    KEY_TAB          = pack_keycode_raw(KeyCode{.right = 0, .down = 2}),
    KEY_Q            = pack_keycode_raw(KeyCode{.right = 1, .down = 2}),
    KEY_W            = pack_keycode_raw(KeyCode{.right = 2, .down = 2}),
    KEY_E            = pack_keycode_raw(KeyCode{.right = 3, .down = 2}),
    KEY_R            = pack_keycode_raw(KeyCode{.right = 4, .down = 2}),
    KEY_T            = pack_keycode_raw(KeyCode{.right = 5, .down = 2}),
    KEY_Y            = pack_keycode_raw(KeyCode{.right = 6, .down = 2}),
    KEY_U            = pack_keycode_raw(KeyCode{.right = 7, .down = 2}),
    KEY_I            = pack_keycode_raw(KeyCode{.right = 8, .down = 2}),
    KEY_O            = pack_keycode_raw(KeyCode{.right = 9, .down = 2}),
    KEY_P            = pack_keycode_raw(KeyCode{.right = 10, .down = 2}),
    KEY_LEFTBRACKET  = pack_keycode_raw(KeyCode{.right = 11, .down = 2}),
    KEY_RIGHTBRACKET = pack_keycode_raw(KeyCode{.right = 12, .down = 2}),
    KEY_BACKSLASH    = pack_keycode_raw(KeyCode{.right = 13, .down = 2}),
    KEY_DELETE       = pack_keycode_raw(KeyCode{.right = 14, .down = 2}),
    KEY_END          = pack_keycode_raw(KeyCode{.right = 15, .down = 2}),
    KEY_PAGEDOWN     = pack_keycode_raw(KeyCode{.right = 16, .down = 2}),
    KEY_KP_7         = pack_keycode_raw(KeyCode{.right = 17, .down = 2}),
    KEY_KP_8         = pack_keycode_raw(KeyCode{.right = 18, .down = 2}),
    KEY_KP_9         = pack_keycode_raw(KeyCode{.right = 19, .down = 2}),
    KEY_KP_PLUS      = pack_keycode_raw(KeyCode{.right = 20, .down = 2}),

    // Row 3 - ASDF row
    KEY_CAPSLOCK  = pack_keycode_raw(KeyCode{.right = 0, .down = 3}),
    KEY_A         = pack_keycode_raw(KeyCode{.right = 1, .down = 3}),
    KEY_S         = pack_keycode_raw(KeyCode{.right = 2, .down = 3}),
    KEY_D         = pack_keycode_raw(KeyCode{.right = 3, .down = 3}),
    KEY_F         = pack_keycode_raw(KeyCode{.right = 4, .down = 3}),
    KEY_G         = pack_keycode_raw(KeyCode{.right = 5, .down = 3}),
    KEY_H         = pack_keycode_raw(KeyCode{.right = 6, .down = 3}),
    KEY_J         = pack_keycode_raw(KeyCode{.right = 7, .down = 3}),
    KEY_K         = pack_keycode_raw(KeyCode{.right = 8, .down = 3}),
    KEY_L         = pack_keycode_raw(KeyCode{.right = 9, .down = 3}),
    KEY_SEMICOLON = pack_keycode_raw(KeyCode{.right = 10, .down = 3}),
    KEY_QUOTE     = pack_keycode_raw(KeyCode{.right = 11, .down = 3}),
    KEY_ENTER     = pack_keycode_raw(KeyCode{.right = 12, .down = 3}),
    KEY_KP_4      = pack_keycode_raw(KeyCode{.right = 13, .down = 3}),
    KEY_KP_5      = pack_keycode_raw(KeyCode{.right = 14, .down = 3}),
    KEY_KP_6      = pack_keycode_raw(KeyCode{.right = 15, .down = 3}),

    // Row 4 - ZXCV row
    KEY_LEFTSHIFT  = pack_keycode_raw(KeyCode{.right = 0, .down = 4}),
    KEY_Z          = pack_keycode_raw(KeyCode{.right = 1, .down = 4}),
    KEY_X          = pack_keycode_raw(KeyCode{.right = 2, .down = 4}),
    KEY_C          = pack_keycode_raw(KeyCode{.right = 3, .down = 4}),
    KEY_V          = pack_keycode_raw(KeyCode{.right = 4, .down = 4}),
    KEY_B          = pack_keycode_raw(KeyCode{.right = 5, .down = 4}),
    KEY_N          = pack_keycode_raw(KeyCode{.right = 6, .down = 4}),
    KEY_M          = pack_keycode_raw(KeyCode{.right = 7, .down = 4}),
    KEY_COMMA      = pack_keycode_raw(KeyCode{.right = 8, .down = 4}),
    KEY_PERIOD     = pack_keycode_raw(KeyCode{.right = 9, .down = 4}),
    KEY_SLASH      = pack_keycode_raw(KeyCode{.right = 10, .down = 4}),
    KEY_RIGHTSHIFT = pack_keycode_raw(KeyCode{.right = 11, .down = 4}),
    KEY_UP         = pack_keycode_raw(KeyCode{.right = 12, .down = 4}),
    KEY_KP_1       = pack_keycode_raw(KeyCode{.right = 13, .down = 4}),
    KEY_KP_2       = pack_keycode_raw(KeyCode{.right = 14, .down = 4}),
    KEY_KP_3       = pack_keycode_raw(KeyCode{.right = 15, .down = 4}),
    KEY_KP_ENTER   = pack_keycode_raw(KeyCode{.right = 16, .down = 4}),

    // Row 5 - Bottom row
    KEY_LEFTCTRL   = pack_keycode_raw(KeyCode{.right = 0, .down = 5}),
    KEY_LEFTMETA   = pack_keycode_raw(KeyCode{.right = 1, .down = 5}),
    KEY_LEFTALT    = pack_keycode_raw(KeyCode{.right = 2, .down = 5}),
    KEY_SPACE      = pack_keycode_raw(KeyCode{.right = 3, .down = 5}),
    KEY_RIGHTALT   = pack_keycode_raw(KeyCode{.right = 4, .down = 5}),
    KEY_RIGHTMETA  = pack_keycode_raw(KeyCode{.right = 5, .down = 5}),
    KEY_MENU       = pack_keycode_raw(KeyCode{.right = 6, .down = 5}),
    KEY_RIGHTCTRL  = pack_keycode_raw(KeyCode{.right = 7, .down = 5}),
    KEY_LEFT       = pack_keycode_raw(KeyCode{.right = 8, .down = 5}),
    KEY_DOWN       = pack_keycode_raw(KeyCode{.right = 9, .down = 5}),
    KEY_RIGHT      = pack_keycode_raw(KeyCode{.right = 10, .down = 5}),
    KEY_KP_0       = pack_keycode_raw(KeyCode{.right = 11, .down = 5}),
    KEY_KP_DECIMAL = pack_keycode_raw(KeyCode{.right = 12, .down = 5}),

    // Row 6 - Extra/media keys
    KEY_POWER           = pack_keycode_raw(KeyCode{.right = 0, .down = 6}),
    KEY_SLEEP           = pack_keycode_raw(KeyCode{.right = 1, .down = 6}),
    KEY_WAKE            = pack_keycode_raw(KeyCode{.right = 2, .down = 6}),
    KEY_MEDIA_PLAY      = pack_keycode_raw(KeyCode{.right = 3, .down = 6}),
    KEY_MEDIA_STOP      = pack_keycode_raw(KeyCode{.right = 4, .down = 6}),
    KEY_MEDIA_PREV      = pack_keycode_raw(KeyCode{.right = 5, .down = 6}),
    KEY_MEDIA_NEXT      = pack_keycode_raw(KeyCode{.right = 6, .down = 6}),
    KEY_VOLUME_UP       = pack_keycode_raw(KeyCode{.right = 7, .down = 6}),
    KEY_VOLUME_DOWN     = pack_keycode_raw(KeyCode{.right = 8, .down = 6}),
    KEY_VOLUME_MUTE     = pack_keycode_raw(KeyCode{.right = 9, .down = 6}),
    KEY_BRIGHTNESS_UP   = pack_keycode_raw(KeyCode{.right = 10, .down = 6}),
    KEY_BRIGHTNESS_DOWN = pack_keycode_raw(KeyCode{.right = 11, .down = 6}),

    KEY_CALCULATOR    = pack_keycode_raw(KeyCode{.right = 12, .down = 6}),
    KEY_WWW_HOME      = pack_keycode_raw(KeyCode{.right = 13, .down = 6}),
    KEY_WWW_SEARCH    = pack_keycode_raw(KeyCode{.right = 14, .down = 6}),
    KEY_WWW_FAVORITES = pack_keycode_raw(KeyCode{.right = 15, .down = 6}),
    KEY_WWW_REFRESH   = pack_keycode_raw(KeyCode{.right = 16, .down = 6}),
    KEY_WWW_STOP      = pack_keycode_raw(KeyCode{.right = 17, .down = 6}),
    KEY_WWW_FORWARD   = pack_keycode_raw(KeyCode{.right = 18, .down = 6}),
    KEY_WWW_BACK      = pack_keycode_raw(KeyCode{.right = 19, .down = 6}),
    KEY_MY_COMPUTER   = pack_keycode_raw(KeyCode{.right = 20, .down = 6}),
    KEY_EMAIL         = pack_keycode_raw(KeyCode{.right = 0, .down = 7}),
    KEY_MEDIA_SELECT  = pack_keycode_raw(KeyCode{.right = 1, .down = 7}),

    // Row 7 - Reserved/future keys
    KEY_F15 = pack_keycode_raw(KeyCode{.right = 2, .down = 7}),
    KEY_F16 = pack_keycode_raw(KeyCode{.right = 3, .down = 7}),
    KEY_F17 = pack_keycode_raw(KeyCode{.right = 4, .down = 7}),
    KEY_F18 = pack_keycode_raw(KeyCode{.right = 5, .down = 7}),
    KEY_F19 = pack_keycode_raw(KeyCode{.right = 6, .down = 7}),
    KEY_F20 = pack_keycode_raw(KeyCode{.right = 7, .down = 7}),
    KEY_F21 = pack_keycode_raw(KeyCode{.right = 8, .down = 7}),
    KEY_F22 = pack_keycode_raw(KeyCode{.right = 9, .down = 7}),
    KEY_F23 = pack_keycode_raw(KeyCode{.right = 10, .down = 7}),
    KEY_F24 = pack_keycode_raw(KeyCode{.right = 11, .down = 7}),

    // Special values
    KEY_NONE    = 0xFF,
    KEY_ANY     = 0xFE,
    KEY_INVALID = 0xFD,
    KEY_UNKNOWN = 0xFC

};

// Helper functions
constexpr KeyCode unpack_keycode(Key key)
{
    uint8_t val = static_cast<uint8_t>(key);
    return KeyCode{.right = static_cast<uint8_t>(val & 0x1F), .down = static_cast<uint8_t>((val >> 5) & 0x07)};
}

constexpr Key pack_keycode(KeyCode keycode)
{
    // multiline
    return static_cast<Key>((static_cast<uint8_t>(keycode.down) << 5) | (keycode.right & 0x1F));
}

constexpr bool is_modifier(Key key)
{
    switch (key)
    {
    case Key::KEY_LEFTSHIFT:
    case Key::KEY_RIGHTSHIFT:
    case Key::KEY_LEFTCTRL:
    case Key::KEY_RIGHTCTRL:
    case Key::KEY_LEFTALT:
    case Key::KEY_RIGHTALT:
    case Key::KEY_LEFTMETA:
    case Key::KEY_RIGHTMETA:
    case Key::KEY_CAPSLOCK:
    case Key::KEY_NUMLOCK:
    case Key::KEY_SCROLLLOCK: return true;
    default: return false;
    }
}

struct ExtraInfo
{
    bool typing_like : 1; // If true, overrides all following fields
                          // If true, behavior is like typing. shift + a = A.
                          // Otherwise, like an application.
                          // Check the other fields
    // Possible this field isn't needed. Since application typing won't work through function list.
    // But through a single function that converts state and keycode to it

    bool dependant_on_capslock : 1;    // if true, capslock must be same state
    bool dependant_on_scroll_lock : 1; // if true, capslock must be the same state.
    // If false, ignore the capslock and scroll lock of KeyModifierState.

    bool first_press_only : 1;
    bool shift_specific : 1;   // if false, any shift will do.
    bool control_specific : 1; // if false any control will do
    bool alt_specific : 1;     // if false, any alt will do.
};

// We will use this for keybindings. A function will first put the keycode, and the mod sate
struct KeyBindingInfo
{
    struct KeyCode          keycode;
    struct KeyModifierState mod_state;
    struct ExtraInfo        toggle_dependancy;
};

void add_application_keybinding(const struct KeyBindingInfo &keybind, void (*keybind_function)(void *args_and_ret));

enum class KeyboardMode : uint8_t
{
    Typing,
    Testing,
    Application,
    TypingApplication,
};
} // namespace keycodes
