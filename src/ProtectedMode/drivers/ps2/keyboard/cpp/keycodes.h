#pragma once
#include "stdio.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Let's use (x, y).
    // where x goes right, and y goes down.

    struct KeyCode
    {
        // 21 (->Right), 6 (v Down)
        // 32 (->Right), 8 down --- Rounded up
        // 5 bits right, 3 for down

        uint8_t right : 5;
        uint8_t down : 3;
    };

#ifdef __cplusplus
    static_assert(sizeof(KeyCode) == 1, "KeyCodes must be 1 byte (8 bits)");
#else
_Static_assert(sizeof(struct KeyCode) == 1, "KeyCodes must be 1 byte (8 bits)");
#endif

    struct KeyModifierState
    {
        bool left_shift : 1;
        bool right_shift : 1;

        bool left_ctrl : 1;
        bool right_ctrl : 1;

        bool left_meta : 1;
        bool right_meta : 1;
        bool menu_app : 1;

        bool alt : 1;
        bool altgr : 1;
        // 8 bits

        // possibly weird behavior for these ones
        bool capslock : 1;
        bool scroll_lock : 1;
    };

    static inline void KeyModifierState_print(const struct KeyModifierState ks)
    {
        kprintf2("Shift(L,R)=%u%u  Ctrl(L,R)=%u%u  Meta(L,R)=%u%u  Menu=%u  Alt=%u  AltGr=%u  Caps=%u  Scroll=%u\n", ks.left_shift,
            ks.right_shift, ks.left_ctrl, ks.right_ctrl, ks.left_meta, ks.right_meta, ks.menu_app, ks.alt, ks.altgr, ks.capslock,
            ks.scroll_lock);
    }

    struct KeyCode scancode_to_keycode(uint8_t scancode);
    struct KeyCode extended_scancode_to_keycode(uint8_t extended_scancode);
    struct KeyCode pause_to_keycode();
    struct KeyCode print_screen_to_keycode();

    void ps2_keyboard_cpp_init();

    // Note that press not released shouldn't be used.
    // But, this is just a test to know if it's known that it's released
    void keycode_handler(struct KeyCode keycode, struct KeyModifierState mod_sate, bool first_press, bool press_not_released);

// End
#ifdef __cplusplus
}
#endif
