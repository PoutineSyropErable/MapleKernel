#include "keycode_to_string.hpp"
#include "keycodes.h"
#include "keycodes.hpp"
#include "keycodes_char.hpp"
#include "scancodes_to_keycodes.hpp"
#include "stdio.h"

keycodes::KeyCharInfo keycodes::KEY_CHAR_TABLE[KEYCODE_COUNT];

using namespace keycodes;
void init_key_char_table()
{
    // Canadian french table

    for (uint16_t i = 0; i < KEYCODE_COUNT; i++)
    {
        KEY_CHAR_TABLE[i] = {0, 0, 0, 0, false, false, false};
    }

    // Numbers Key (0-9)
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_1)] = {'1', '!', '1', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_2)] = {'2', '"', '2', '@', false, true, true};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_3)] = {'3', '/', '3', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_4)] = {'4', '$', '4', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_5)] = {'5', '%', '5', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_6)] = {'6', '?', '6', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_7)] = {'7', '&', '7', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_8)] = {'8', '*', '8', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_9)] = {'9', '(', '9', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_0)] = {'0', ')', '0', ' ', false, true, false};

    // Letters A-Z
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_A)] = {'a', 'A', 'A', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_B)] = {'b', 'B', 'B', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_C)] = {'c', 'C', 'C', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_D)] = {'d', 'D', 'D', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_E)] = {'e', 'E', 'E', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_F)] = {'f', 'F', 'F', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_G)] = {'g', 'G', 'G', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_H)] = {'h', 'H', 'H', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_I)] = {'i', 'I', 'I', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_J)] = {'j', 'J', 'J', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_K)] = {'k', 'K', 'K', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_L)] = {'l', 'L', 'L', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_M)] = {'m', 'M', 'M', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_N)] = {'n', 'N', 'N', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_O)] = {'o', 'O', 'O', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_P)] = {'p', 'P', 'P', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_Q)] = {'q', 'Q', 'Q', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_R)] = {'r', 'R', 'R', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_S)] = {'s', 'S', 'S', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_T)] = {'t', 'T', 'T', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_U)] = {'u', 'U', 'U', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_V)] = {'v', 'V', 'V', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_W)] = {'w', 'W', 'W', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_X)] = {'x', 'X', 'X', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_Y)] = {'y', 'Y', 'Y', ' ', true, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_Z)] = {'z', 'Z', 'Z', ' ', true, true, false};

    // Punctuation
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_BACKTICK)]     = {'#', '|', '#', '\\', false, true, true};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_MINUS)]        = {'-', '_', '-', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_EQUALS)]       = {'=', '+', '=', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_LEFTBRACKET)]  = {'^', '^', '^', '[', false, true, true};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_RIGHTBRACKET)] = {']', ']', ']', ']', false, true, true};
    // ^ Hack. Since its only algr printable
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_BACKSLASH)] = {'<', '>', '<', '}', false, true, true};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_SEMICOLON)] = {';', ':', ';', '~', false, true, true};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_QUOTE)]     = {'`', '`', '`', '{', false, true, true};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_COMMA)]     = {',', '\'', ',', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_PERIOD)]    = {'.', '.', '.', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_SLASH)]     = {'/', '/', '/', ' ', false, true, false};
    // Slash is useful, so im not gonna have the ca fr thing on it.

    // Whitespace
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_SPACE)] = {' ', ' ', ' ', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_TAB)]   = {'\t', '\t', '\t', ' ', false, true, false};
    KEY_CHAR_TABLE[static_cast<uint8_t>(Key::KEY_ENTER)] = {'\n', '\n', '\n', ' ', false, true, false};
}

struct KeyCode scancode_to_keycode(uint8_t scancode)
{
    // multiline
    return keycodes::ScanToKeyCode[scancode];
}
struct KeyCode extended_scancode_to_keycode(uint8_t extended_scancode)
{
    // multiline
    return keycodes::ExtendedScanToKeyCode[extended_scancode];
}
struct KeyCode pause_to_keycode()
{
    return unpack_keycode(Key::KEY_PAUSE);
}
struct KeyCode print_screen_to_keycode()
{
    return unpack_keycode(Key::KEY_PRINTSCR);
}

KeyboardMode keyboard_mode;

void ps2_keyboard_cpp_init()
{
    // This should all be comptime, but sadly, i can't.
    init_key_char_table();
    init_scancodes_to_keycodes();
    initialize_keycodes_name_converter();

    keyboard_mode = keycodes::KeyboardMode::Testing;
    // keyboard_mode = keycodes::KeyboardMode::Typing;
}

void keycode_handler(struct KeyCode keycode, struct KeyModifierState mod_state, bool first_press, bool press_not_released)
{
    Key key = keycodes::pack_keycode(keycode);
    if (keyboard_mode == keycodes::KeyboardMode::Testing)
    {

        if (key == Key::KEY_INVALID || key == Key::KEY_NONE || key == Key::KEY_UNKNOWN)
        {
            kprintf2("[Invalid     ] keycode = (%u, %u) = %s\n", keycode.down, keycode.right, keycodes::key_to_string(key));
        }
        else if (first_press)
        {
            kprintf2("[FIRST PRESS ] keycode = (%u, %u) = %s\n", keycode.down, keycode.right, keycodes::key_to_string(key));
        }
        else if (press_not_released)
        {
            kprintf2("[REPEAT PRESS] keycode = (%u, %u) = %s\n", keycode.down, keycode.right, keycodes::key_to_string(key));
        }
        else
        {
            kprintf2("[Release     ] keycode = (%u, %u) = %s\n", keycode.down, keycode.right, keycodes::key_to_string(key));
        }
    }
    else if (keyboard_mode == keycodes::KeyboardMode::Typing)
    {

        if (!press_not_released)
            return;

        bool letter     = is_letter(key);
        bool altgr_able = is_altgr_able(key);
        bool printable  = is_printable(key);

        // KeyModifierState_print(mod_state);
        if (printable)
        {
            char write_char = key_to_char(key, mod_state);
            kprintf2("%c", write_char);
        }
    }
    else if (keyboard_mode == keycodes::KeyboardMode::Application)
    {
    }
}
