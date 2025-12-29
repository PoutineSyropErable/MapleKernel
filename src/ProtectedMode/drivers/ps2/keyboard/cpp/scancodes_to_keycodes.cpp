#include "keycode_to_string.hpp"
#include "keycodes.hpp"
#include "scancodes_to_keycodes.hpp"

#include "extended_scancode.h"
#include "scancode.h"

KeyCode keycodes::ScanToKeyCode[SCANCODE_COUNT];
KeyCode keycodes::ExtendedScanToKeyCode[SCANCODE_COUNT];

using namespace keycodes;

void keycodes::init_scancodes_to_keycodes()
{
    // placeholder
    for (uint8_t i = 0; i < keycodes::SCANCODE_COUNT; i++)
    {
        ExtendedScanToKeyCode[i] = unpack_keycode(Key::KEY_INVALID);
        ScanToKeyCode[i]         = unpack_keycode(Key::KEY_INVALID);
    }

    // Row 0 - Escape + Functions
    ScanToKeyCode[SC_ESCAPE] = unpack_keycode(Key::KEY_ESC);
    ScanToKeyCode[SC_F1]     = unpack_keycode(Key::KEY_F1);
    ScanToKeyCode[SC_F2]     = unpack_keycode(Key::KEY_F2);
    ScanToKeyCode[SC_F3]     = unpack_keycode(Key::KEY_F3);
    ScanToKeyCode[SC_F4]     = unpack_keycode(Key::KEY_F4);
    ScanToKeyCode[SC_F5]     = unpack_keycode(Key::KEY_F5);
    ScanToKeyCode[SC_F6]     = unpack_keycode(Key::KEY_F6);
    ScanToKeyCode[SC_F7]     = unpack_keycode(Key::KEY_F7);
    ScanToKeyCode[SC_F8]     = unpack_keycode(Key::KEY_F8);
    ScanToKeyCode[SC_F9]     = unpack_keycode(Key::KEY_F9);
    ScanToKeyCode[SC_F10]    = unpack_keycode(Key::KEY_F10);
    ScanToKeyCode[SC_F11]    = unpack_keycode(Key::KEY_F11);
    ScanToKeyCode[SC_F12]    = unpack_keycode(Key::KEY_F12);
    // Printscreen
    ScanToKeyCode[SC_SCROLLLOCK] = unpack_keycode(Key::KEY_SCROLLLOCK);
    // Pause
    // ===== ^^Printscreen and pause are implemented using a different function

    // Row 1 - Numbers
    ScanToKeyCode[SC_BACKTICK]          = unpack_keycode(Key::KEY_BACKTICK);
    ScanToKeyCode[SC_1]                 = unpack_keycode(Key::KEY_1);
    ScanToKeyCode[SC_2]                 = unpack_keycode(Key::KEY_2);
    ScanToKeyCode[SC_3]                 = unpack_keycode(Key::KEY_3);
    ScanToKeyCode[SC_4]                 = unpack_keycode(Key::KEY_4);
    ScanToKeyCode[SC_5]                 = unpack_keycode(Key::KEY_5);
    ScanToKeyCode[SC_6]                 = unpack_keycode(Key::KEY_6);
    ScanToKeyCode[SC_7]                 = unpack_keycode(Key::KEY_7);
    ScanToKeyCode[SC_8]                 = unpack_keycode(Key::KEY_8);
    ScanToKeyCode[SC_9]                 = unpack_keycode(Key::KEY_9);
    ScanToKeyCode[SC_0]                 = unpack_keycode(Key::KEY_0);
    ScanToKeyCode[SC_MINUS]             = unpack_keycode(Key::KEY_MINUS);
    ScanToKeyCode[SC_EQUALS]            = unpack_keycode(Key::KEY_EQUALS);
    ScanToKeyCode[SC_BACKSPACE]         = unpack_keycode(Key::KEY_BACKSPACE);
    ExtendedScanToKeyCode[SCE_INSERT]   = unpack_keycode(Key::KEY_INSERT);
    ExtendedScanToKeyCode[SCE_HOME]     = unpack_keycode(Key::KEY_HOME);
    ExtendedScanToKeyCode[SCE_PAGE_UP]  = unpack_keycode(Key::KEY_PAGEUP);
    ScanToKeyCode[SC_NUMLOCK]           = unpack_keycode(Key::KEY_NUMLOCK);
    ExtendedScanToKeyCode[SCE_KP_SLASH] = unpack_keycode(Key::KEY_KP_DIVIDE);
    ScanToKeyCode[SC_KP_ASTERISK]       = unpack_keycode(Key::KEY_KP_MULTIPLY);
    ScanToKeyCode[SC_KP_MINUS]          = unpack_keycode(Key::KEY_KP_MINUS);

    // Row 2. QWERTY  row
    ScanToKeyCode[SC_TAB]                = unpack_keycode(Key::KEY_TAB);
    ScanToKeyCode[SC_Q]                  = unpack_keycode(Key::KEY_Q);
    ScanToKeyCode[SC_W]                  = unpack_keycode(Key::KEY_W);
    ScanToKeyCode[SC_E]                  = unpack_keycode(Key::KEY_E);
    ScanToKeyCode[SC_R]                  = unpack_keycode(Key::KEY_R);
    ScanToKeyCode[SC_T]                  = unpack_keycode(Key::KEY_T);
    ScanToKeyCode[SC_Y]                  = unpack_keycode(Key::KEY_Y);
    ScanToKeyCode[SC_U]                  = unpack_keycode(Key::KEY_U);
    ScanToKeyCode[SC_I]                  = unpack_keycode(Key::KEY_I);
    ScanToKeyCode[SC_O]                  = unpack_keycode(Key::KEY_O);
    ScanToKeyCode[SC_P]                  = unpack_keycode(Key::KEY_P);
    ScanToKeyCode[SC_LBRACKET]           = unpack_keycode(Key::KEY_LEFTBRACKET);
    ScanToKeyCode[SC_RBRACKET]           = unpack_keycode(Key::KEY_RIGHTBRACKET);
    ScanToKeyCode[SC_BACKSLASH]          = unpack_keycode(Key::KEY_BACKSLASH);
    ExtendedScanToKeyCode[SCE_DELETE]    = unpack_keycode(Key::KEY_DELETE);
    ExtendedScanToKeyCode[SCE_END]       = unpack_keycode(Key::KEY_END);
    ExtendedScanToKeyCode[SCE_PAGE_DOWN] = unpack_keycode(Key::KEY_PAGEDOWN);
    ScanToKeyCode[SC_KP_7]               = unpack_keycode(Key::KEY_KP_7);
    ScanToKeyCode[SC_KP_8]               = unpack_keycode(Key::KEY_KP_8);
    ScanToKeyCode[SC_KP_9]               = unpack_keycode(Key::KEY_KP_9);
    ScanToKeyCode[SC_KP_PLUS]            = unpack_keycode(Key::KEY_KP_PLUS);

    // Row 3 - ASDF row
    ScanToKeyCode[SC_CAPSLOCK]  = unpack_keycode(Key::KEY_CAPSLOCK);
    ScanToKeyCode[SC_A]         = unpack_keycode(Key::KEY_A);
    ScanToKeyCode[SC_S]         = unpack_keycode(Key::KEY_S);
    ScanToKeyCode[SC_D]         = unpack_keycode(Key::KEY_D);
    ScanToKeyCode[SC_F]         = unpack_keycode(Key::KEY_F);
    ScanToKeyCode[SC_G]         = unpack_keycode(Key::KEY_G);
    ScanToKeyCode[SC_H]         = unpack_keycode(Key::KEY_H);
    ScanToKeyCode[SC_J]         = unpack_keycode(Key::KEY_J);
    ScanToKeyCode[SC_K]         = unpack_keycode(Key::KEY_K);
    ScanToKeyCode[SC_L]         = unpack_keycode(Key::KEY_L);
    ScanToKeyCode[SC_SEMICOLON] = unpack_keycode(Key::KEY_SEMICOLON);
    ScanToKeyCode[SC_QUOTE]     = unpack_keycode(Key::KEY_QUOTE);
    ScanToKeyCode[SC_ENTER]     = unpack_keycode(Key::KEY_ENTER);
    ScanToKeyCode[SC_KP_4]      = unpack_keycode(Key::KEY_KP_4);
    ScanToKeyCode[SC_KP_5]      = unpack_keycode(Key::KEY_KP_5);
    ScanToKeyCode[SC_KP_6]      = unpack_keycode(Key::KEY_KP_6);

    // Row 4 - ZXCV row
    ScanToKeyCode[SC_LEFT_SHIFT]        = unpack_keycode(Key::KEY_LEFTSHIFT);
    ScanToKeyCode[SC_Z]                 = unpack_keycode(Key::KEY_Z);
    ScanToKeyCode[SC_X_]                = unpack_keycode(Key::KEY_X);
    ScanToKeyCode[SC_C]                 = unpack_keycode(Key::KEY_C);
    ScanToKeyCode[SC_V]                 = unpack_keycode(Key::KEY_V);
    ScanToKeyCode[SC_B]                 = unpack_keycode(Key::KEY_B);
    ScanToKeyCode[SC_N]                 = unpack_keycode(Key::KEY_N);
    ScanToKeyCode[SC_M]                 = unpack_keycode(Key::KEY_M);
    ScanToKeyCode[SC_COMMA]             = unpack_keycode(Key::KEY_COMMA);
    ScanToKeyCode[SC_DOT]               = unpack_keycode(Key::KEY_PERIOD);
    ScanToKeyCode[SC_SLASH]             = unpack_keycode(Key::KEY_SLASH);
    ScanToKeyCode[SC_RIGHT_SHIFT]       = unpack_keycode(Key::KEY_RIGHTSHIFT);
    ExtendedScanToKeyCode[SCE_UP]       = unpack_keycode(Key::KEY_UP);
    ScanToKeyCode[SC_KP_1]              = unpack_keycode(Key::KEY_KP_1);
    ScanToKeyCode[SC_KP_2]              = unpack_keycode(Key::KEY_KP_2);
    ScanToKeyCode[SC_KP_3]              = unpack_keycode(Key::KEY_KP_3);
    ExtendedScanToKeyCode[SCE_KP_ENTER] = unpack_keycode(Key::KEY_KP_ENTER);

    // Row 5 - Bottom row
    ScanToKeyCode[SC_LEFT_CTRL]           = unpack_keycode(Key::KEY_LEFTCTRL);
    ExtendedScanToKeyCode[SCE_LEFT_META]  = unpack_keycode(Key::KEY_LEFTMETA);
    ScanToKeyCode[SC_LEFT_ALT]            = unpack_keycode(Key::KEY_LEFTALT);
    ScanToKeyCode[SC_SPACE]               = unpack_keycode(Key::KEY_SPACE);
    ExtendedScanToKeyCode[SCE_RIGHT_ALT]  = unpack_keycode(Key::KEY_RIGHTALT);
    ExtendedScanToKeyCode[SCE_RIGHT_META] = unpack_keycode(Key::KEY_RIGHTMETA);
    ExtendedScanToKeyCode[SCE_APPS]       = unpack_keycode(Key::KEY_MENU);
    ExtendedScanToKeyCode[SCE_RIGHT_CTRL] = unpack_keycode(Key::KEY_RIGHTCTRL);
    ExtendedScanToKeyCode[SCE_LEFT]       = unpack_keycode(Key::KEY_LEFT);
    ExtendedScanToKeyCode[SCE_DOWN]       = unpack_keycode(Key::KEY_DOWN);
    ExtendedScanToKeyCode[SCE_RIGHT]      = unpack_keycode(Key::KEY_RIGHT);
    ScanToKeyCode[SC_KP_0]                = unpack_keycode(Key::KEY_KP_0);
    ScanToKeyCode[SC_KP_DOT]              = unpack_keycode(Key::KEY_KP_DECIMAL);

    // Other extended
    ExtendedScanToKeyCode[SCE_PREV_TRACK]    = unpack_keycode(Key::KEY_MEDIA_PREV);
    ExtendedScanToKeyCode[SCE_NEXT_TRACK]    = unpack_keycode(Key::KEY_MEDIA_NEXT);
    ExtendedScanToKeyCode[SCE_PLAY]          = unpack_keycode(Key::KEY_MEDIA_PLAY);
    ExtendedScanToKeyCode[SCE_STOP]          = unpack_keycode(Key::KEY_MEDIA_STOP);
    ExtendedScanToKeyCode[SCE_MUTE]          = unpack_keycode(Key::KEY_VOLUME_MUTE);
    ExtendedScanToKeyCode[SCE_VOLUME_UP]     = unpack_keycode(Key::KEY_VOLUME_UP);
    ExtendedScanToKeyCode[SCE_VOLUME_DOWN]   = unpack_keycode(Key::KEY_VOLUME_DOWN);
    ExtendedScanToKeyCode[SCE_CALCULATOR]    = unpack_keycode(Key::KEY_CALCULATOR);
    ExtendedScanToKeyCode[SCE_WWW_HOME]      = unpack_keycode(Key::KEY_WWW_HOME);
    ExtendedScanToKeyCode[SCE_WWW_SEARCH]    = unpack_keycode(Key::KEY_WWW_SEARCH);
    ExtendedScanToKeyCode[SCE_WWW_FAVORITES] = unpack_keycode(Key::KEY_WWW_FAVORITES);
    ExtendedScanToKeyCode[SCE_WWW_REFRESH]   = unpack_keycode(Key::KEY_WWW_REFRESH);
    ExtendedScanToKeyCode[SCE_WWW_STOP]      = unpack_keycode(Key::KEY_WWW_STOP);
    ExtendedScanToKeyCode[SCE_WWW_FORWARD]   = unpack_keycode(Key::KEY_WWW_FORWARD);
    ExtendedScanToKeyCode[SCE_WWW_BACK]      = unpack_keycode(Key::KEY_WWW_BACK);
    ExtendedScanToKeyCode[SCE_MY_COMPUTER]   = unpack_keycode(Key::KEY_MY_COMPUTER);
    ExtendedScanToKeyCode[SCE_EMAIL]         = unpack_keycode(Key::KEY_EMAIL);
    ExtendedScanToKeyCode[SCE_MEDIA_SELECT]  = unpack_keycode(Key::KEY_MEDIA_SELECT);

    // Power / system keys
    ExtendedScanToKeyCode[SCE_POWER] = unpack_keycode(Key::KEY_POWER);
    ExtendedScanToKeyCode[SCE_SLEEP] = unpack_keycode(Key::KEY_SLEEP);
    ExtendedScanToKeyCode[SCE_WAKE]  = unpack_keycode(Key::KEY_WAKE);
}

const char *keycodes::key_strings[256] = {nullptr};

void keycodes::initialize_keycodes_name_converter()
{
    for (int i = 0; i < 256; ++i)
        key_strings[i] = "UNKNOWN";

    // Function keys and special keys
    key_strings[static_cast<uint8_t>(Key::KEY_ESC)]        = "KEY_ESC";
    key_strings[static_cast<uint8_t>(Key::KEY_F1)]         = "KEY_F1";
    key_strings[static_cast<uint8_t>(Key::KEY_F2)]         = "KEY_F2";
    key_strings[static_cast<uint8_t>(Key::KEY_F3)]         = "KEY_F3";
    key_strings[static_cast<uint8_t>(Key::KEY_F4)]         = "KEY_F4";
    key_strings[static_cast<uint8_t>(Key::KEY_F5)]         = "KEY_F5";
    key_strings[static_cast<uint8_t>(Key::KEY_F6)]         = "KEY_F6";
    key_strings[static_cast<uint8_t>(Key::KEY_F7)]         = "KEY_F7";
    key_strings[static_cast<uint8_t>(Key::KEY_F8)]         = "KEY_F8";
    key_strings[static_cast<uint8_t>(Key::KEY_F9)]         = "KEY_F9";
    key_strings[static_cast<uint8_t>(Key::KEY_F10)]        = "KEY_F10";
    key_strings[static_cast<uint8_t>(Key::KEY_F11)]        = "KEY_F11";
    key_strings[static_cast<uint8_t>(Key::KEY_F12)]        = "KEY_F12";
    key_strings[static_cast<uint8_t>(Key::KEY_PRINTSCR)]   = "KEY_PRINTSCR";
    key_strings[static_cast<uint8_t>(Key::KEY_SCROLLLOCK)] = "KEY_SCROLLLOCK";
    key_strings[static_cast<uint8_t>(Key::KEY_PAUSE)]      = "KEY_PAUSE";

    // Row 1 - Number row
    key_strings[static_cast<uint8_t>(Key::KEY_BACKTICK)]    = "KEY_BACKTICK";
    key_strings[static_cast<uint8_t>(Key::KEY_1)]           = "KEY_1";
    key_strings[static_cast<uint8_t>(Key::KEY_2)]           = "KEY_2";
    key_strings[static_cast<uint8_t>(Key::KEY_3)]           = "KEY_3";
    key_strings[static_cast<uint8_t>(Key::KEY_4)]           = "KEY_4";
    key_strings[static_cast<uint8_t>(Key::KEY_5)]           = "KEY_5";
    key_strings[static_cast<uint8_t>(Key::KEY_6)]           = "KEY_6";
    key_strings[static_cast<uint8_t>(Key::KEY_7)]           = "KEY_7";
    key_strings[static_cast<uint8_t>(Key::KEY_8)]           = "KEY_8";
    key_strings[static_cast<uint8_t>(Key::KEY_9)]           = "KEY_9";
    key_strings[static_cast<uint8_t>(Key::KEY_0)]           = "KEY_0";
    key_strings[static_cast<uint8_t>(Key::KEY_MINUS)]       = "KEY_MINUS";
    key_strings[static_cast<uint8_t>(Key::KEY_EQUALS)]      = "KEY_EQUALS";
    key_strings[static_cast<uint8_t>(Key::KEY_BACKSPACE)]   = "KEY_BACKSPACE";
    key_strings[static_cast<uint8_t>(Key::KEY_INSERT)]      = "KEY_INSERT";
    key_strings[static_cast<uint8_t>(Key::KEY_HOME)]        = "KEY_HOME";
    key_strings[static_cast<uint8_t>(Key::KEY_PAGEUP)]      = "KEY_PAGEUP";
    key_strings[static_cast<uint8_t>(Key::KEY_NUMLOCK)]     = "KEY_NUMLOCK";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_DIVIDE)]   = "KEY_KP_DIVIDE";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_MULTIPLY)] = "KEY_KP_MULTIPLY";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_MINUS)]    = "KEY_KP_MINUS";

    // Row 2 - QWERTY row
    key_strings[static_cast<uint8_t>(Key::KEY_TAB)]          = "KEY_TAB";
    key_strings[static_cast<uint8_t>(Key::KEY_Q)]            = "KEY_Q";
    key_strings[static_cast<uint8_t>(Key::KEY_W)]            = "KEY_W";
    key_strings[static_cast<uint8_t>(Key::KEY_E)]            = "KEY_E";
    key_strings[static_cast<uint8_t>(Key::KEY_R)]            = "KEY_R";
    key_strings[static_cast<uint8_t>(Key::KEY_T)]            = "KEY_T";
    key_strings[static_cast<uint8_t>(Key::KEY_Y)]            = "KEY_Y";
    key_strings[static_cast<uint8_t>(Key::KEY_U)]            = "KEY_U";
    key_strings[static_cast<uint8_t>(Key::KEY_I)]            = "KEY_I";
    key_strings[static_cast<uint8_t>(Key::KEY_O)]            = "KEY_O";
    key_strings[static_cast<uint8_t>(Key::KEY_P)]            = "KEY_P";
    key_strings[static_cast<uint8_t>(Key::KEY_LEFTBRACKET)]  = "KEY_LEFTBRACKET";
    key_strings[static_cast<uint8_t>(Key::KEY_RIGHTBRACKET)] = "KEY_RIGHTBRACKET";
    key_strings[static_cast<uint8_t>(Key::KEY_BACKSLASH)]    = "KEY_BACKSLASH";
    key_strings[static_cast<uint8_t>(Key::KEY_DELETE)]       = "KEY_DELETE";
    key_strings[static_cast<uint8_t>(Key::KEY_END)]          = "KEY_END";
    key_strings[static_cast<uint8_t>(Key::KEY_PAGEDOWN)]     = "KEY_PAGEDOWN";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_7)]         = "KEY_KP_7";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_8)]         = "KEY_KP_8";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_9)]         = "KEY_KP_9";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_PLUS)]      = "KEY_KP_PLUS";

    // Row 3 - ASDF row
    key_strings[static_cast<uint8_t>(Key::KEY_CAPSLOCK)]  = "KEY_CAPSLOCK";
    key_strings[static_cast<uint8_t>(Key::KEY_A)]         = "KEY_A";
    key_strings[static_cast<uint8_t>(Key::KEY_S)]         = "KEY_S";
    key_strings[static_cast<uint8_t>(Key::KEY_D)]         = "KEY_D";
    key_strings[static_cast<uint8_t>(Key::KEY_F)]         = "KEY_F";
    key_strings[static_cast<uint8_t>(Key::KEY_G)]         = "KEY_G";
    key_strings[static_cast<uint8_t>(Key::KEY_H)]         = "KEY_H";
    key_strings[static_cast<uint8_t>(Key::KEY_J)]         = "KEY_J";
    key_strings[static_cast<uint8_t>(Key::KEY_K)]         = "KEY_K";
    key_strings[static_cast<uint8_t>(Key::KEY_L)]         = "KEY_L";
    key_strings[static_cast<uint8_t>(Key::KEY_SEMICOLON)] = "KEY_SEMICOLON";
    key_strings[static_cast<uint8_t>(Key::KEY_QUOTE)]     = "KEY_QUOTE";
    key_strings[static_cast<uint8_t>(Key::KEY_ENTER)]     = "KEY_ENTER";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_4)]      = "KEY_KP_4";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_5)]      = "KEY_KP_5";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_6)]      = "KEY_KP_6";

    // Row 4 - ZXCV row
    key_strings[static_cast<uint8_t>(Key::KEY_LEFTSHIFT)]  = "KEY_LEFTSHIFT";
    key_strings[static_cast<uint8_t>(Key::KEY_Z)]          = "KEY_Z";
    key_strings[static_cast<uint8_t>(Key::KEY_X)]          = "KEY_X";
    key_strings[static_cast<uint8_t>(Key::KEY_C)]          = "KEY_C";
    key_strings[static_cast<uint8_t>(Key::KEY_V)]          = "KEY_V";
    key_strings[static_cast<uint8_t>(Key::KEY_B)]          = "KEY_B";
    key_strings[static_cast<uint8_t>(Key::KEY_N)]          = "KEY_N";
    key_strings[static_cast<uint8_t>(Key::KEY_M)]          = "KEY_M";
    key_strings[static_cast<uint8_t>(Key::KEY_COMMA)]      = "KEY_COMMA";
    key_strings[static_cast<uint8_t>(Key::KEY_PERIOD)]     = "KEY_PERIOD";
    key_strings[static_cast<uint8_t>(Key::KEY_SLASH)]      = "KEY_SLASH";
    key_strings[static_cast<uint8_t>(Key::KEY_RIGHTSHIFT)] = "KEY_RIGHTSHIFT";
    key_strings[static_cast<uint8_t>(Key::KEY_UP)]         = "KEY_UP";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_1)]       = "KEY_KP_1";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_2)]       = "KEY_KP_2";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_3)]       = "KEY_KP_3";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_ENTER)]   = "KEY_KP_ENTER";

    // Row 5 - Bottom row
    key_strings[static_cast<uint8_t>(Key::KEY_LEFTCTRL)]   = "KEY_LEFTCTRL";
    key_strings[static_cast<uint8_t>(Key::KEY_LEFTMETA)]   = "KEY_LEFTMETA";
    key_strings[static_cast<uint8_t>(Key::KEY_LEFTALT)]    = "KEY_LEFTALT";
    key_strings[static_cast<uint8_t>(Key::KEY_SPACE)]      = "KEY_SPACE";
    key_strings[static_cast<uint8_t>(Key::KEY_RIGHTALT)]   = "KEY_RIGHTALT";
    key_strings[static_cast<uint8_t>(Key::KEY_RIGHTMETA)]  = "KEY_RIGHTMETA";
    key_strings[static_cast<uint8_t>(Key::KEY_MENU)]       = "KEY_MENU";
    key_strings[static_cast<uint8_t>(Key::KEY_RIGHTCTRL)]  = "KEY_RIGHTCTRL";
    key_strings[static_cast<uint8_t>(Key::KEY_LEFT)]       = "KEY_LEFT";
    key_strings[static_cast<uint8_t>(Key::KEY_DOWN)]       = "KEY_DOWN";
    key_strings[static_cast<uint8_t>(Key::KEY_RIGHT)]      = "KEY_RIGHT";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_0)]       = "KEY_KP_0";
    key_strings[static_cast<uint8_t>(Key::KEY_KP_DECIMAL)] = "KEY_KP_DECIMAL";

    // Row 6 - Extra/media keys
    key_strings[static_cast<uint8_t>(Key::KEY_POWER)]           = "KEY_POWER";
    key_strings[static_cast<uint8_t>(Key::KEY_SLEEP)]           = "KEY_SLEEP";
    key_strings[static_cast<uint8_t>(Key::KEY_WAKE)]            = "KEY_WAKE";
    key_strings[static_cast<uint8_t>(Key::KEY_MEDIA_PLAY)]      = "KEY_MEDIA_PLAY";
    key_strings[static_cast<uint8_t>(Key::KEY_MEDIA_STOP)]      = "KEY_MEDIA_STOP";
    key_strings[static_cast<uint8_t>(Key::KEY_MEDIA_PREV)]      = "KEY_MEDIA_PREV";
    key_strings[static_cast<uint8_t>(Key::KEY_MEDIA_NEXT)]      = "KEY_MEDIA_NEXT";
    key_strings[static_cast<uint8_t>(Key::KEY_VOLUME_UP)]       = "KEY_VOLUME_UP";
    key_strings[static_cast<uint8_t>(Key::KEY_VOLUME_DOWN)]     = "KEY_VOLUME_DOWN";
    key_strings[static_cast<uint8_t>(Key::KEY_VOLUME_MUTE)]     = "KEY_VOLUME_MUTE";
    key_strings[static_cast<uint8_t>(Key::KEY_BRIGHTNESS_UP)]   = "KEY_BRIGHTNESS_UP";
    key_strings[static_cast<uint8_t>(Key::KEY_BRIGHTNESS_DOWN)] = "KEY_BRIGHTNESS_DOWN";

    key_strings[static_cast<uint8_t>(Key::KEY_CALCULATOR)]    = "KEY_CALCULATOR";
    key_strings[static_cast<uint8_t>(Key::KEY_WWW_HOME)]      = "KEY_WWW_HOME";
    key_strings[static_cast<uint8_t>(Key::KEY_WWW_SEARCH)]    = "KEY_WWW_SEARCH";
    key_strings[static_cast<uint8_t>(Key::KEY_WWW_FAVORITES)] = "KEY_WWW_FAVORITES";
    key_strings[static_cast<uint8_t>(Key::KEY_WWW_REFRESH)]   = "KEY_WWW_REFRESH";
    key_strings[static_cast<uint8_t>(Key::KEY_WWW_STOP)]      = "KEY_WWW_STOP";
    key_strings[static_cast<uint8_t>(Key::KEY_WWW_FORWARD)]   = "KEY_WWW_FORWARD";
    key_strings[static_cast<uint8_t>(Key::KEY_WWW_BACK)]      = "KEY_WWW_BACK";
    key_strings[static_cast<uint8_t>(Key::KEY_MY_COMPUTER)]   = "KEY_MY_COMPUTER";
    key_strings[static_cast<uint8_t>(Key::KEY_EMAIL)]         = "KEY_EMAIL";
    key_strings[static_cast<uint8_t>(Key::KEY_MEDIA_SELECT)]  = "KEY_MEDIA_SELECT";

    // Row 7 - Reserved/future keys
    key_strings[static_cast<uint8_t>(Key::KEY_F15)] = "KEY_F15";
    key_strings[static_cast<uint8_t>(Key::KEY_F16)] = "KEY_F16";
    key_strings[static_cast<uint8_t>(Key::KEY_F17)] = "KEY_F17";
    key_strings[static_cast<uint8_t>(Key::KEY_F18)] = "KEY_F18";
    key_strings[static_cast<uint8_t>(Key::KEY_F19)] = "KEY_F19";
    key_strings[static_cast<uint8_t>(Key::KEY_F20)] = "KEY_F20";
    key_strings[static_cast<uint8_t>(Key::KEY_F21)] = "KEY_F21";
    key_strings[static_cast<uint8_t>(Key::KEY_F22)] = "KEY_F22";
    key_strings[static_cast<uint8_t>(Key::KEY_F23)] = "KEY_F23";
    key_strings[static_cast<uint8_t>(Key::KEY_F24)] = "KEY_F24";

    // Special values
    key_strings[static_cast<uint8_t>(Key::KEY_NONE)]    = "KEY_NONE";
    key_strings[static_cast<uint8_t>(Key::KEY_ANY)]     = "KEY_ANY";
    key_strings[static_cast<uint8_t>(Key::KEY_INVALID)] = "KEY_INVALID";
    key_strings[static_cast<uint8_t>(Key::KEY_UNKNOWN)] = "KEY_UNKNOWN";
}
