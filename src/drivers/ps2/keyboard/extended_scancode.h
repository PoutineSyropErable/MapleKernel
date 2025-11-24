#pragma once
#include <stddef.h>
#include <stdint.h>

// Extended scan codes (2-byte sequences, second byte only for enum)
#define EXTENDED_SCANCODES(X)      \
	/* Multimedia keys pressed */  \
	X(PREV_TRACK, 0x10)            \
	X(NEXT_TRACK, 0x19)            \
	X(KP_ENTER, 0x1C)              \
	X(RIGHT_CTRL, 0x1D)            \
	X(MUTE, 0x20)                  \
	X(CALCULATOR, 0x21)            \
	X(PLAY, 0x22)                  \
	X(STOP, 0x24)                  \
	X(VOLUME_DOWN, 0x2E)           \
	X(VOLUME_UP, 0x30)             \
	X(WWW_HOME, 0x32)              \
	X(KP_SLASH, 0x35)              \
	X(RIGHT_ALT, 0x38)             \
	X(HOME, 0x47)                  \
	X(UP, 0x48)                    \
	X(PAGE_UP, 0x49)               \
	X(LEFT, 0x4B)                  \
	X(RIGHT, 0x4D)                 \
	X(END, 0x4F)                   \
	X(DOWN, 0x50)                  \
	X(PAGE_DOWN, 0x51)             \
	X(INSERT, 0x52)                \
	X(DELETE, 0x53)                \
	X(LEFT_GUI, 0x5B)              \
	X(RIGHT_GUI, 0x5C)             \
	X(APPS, 0x5D)                  \
	X(POWER, 0x5E)                 \
	X(SLEEP, 0x5F)                 \
	X(WAKE, 0x63)                  \
	X(WWW_SEARCH, 0x65)            \
	X(WWW_FAVORITES, 0x66)         \
	X(WWW_REFRESH, 0x67)           \
	X(WWW_STOP, 0x68)              \
	X(WWW_FORWARD, 0x69)           \
	X(WWW_BACK, 0x6A)              \
	X(MY_COMPUTER, 0x6B)           \
	X(EMAIL, 0x6C)                 \
	X(MEDIA_SELECT, 0x6D)          \
                                   \
	/* Multimedia keys released */ \
	X(PREV_TRACK_R, 0x90)          \
	X(NEXT_TRACK_R, 0x99)          \
	X(KP_ENTER_R, 0x9C)            \
	X(RIGHT_CTRL_R, 0x9D)          \
	X(MUTE_R, 0xA0)                \
	X(CALCULATOR_R, 0xA1)          \
	X(PLAY_R, 0xA2)                \
	X(STOP_R, 0xA4)                \
	X(VOLUME_DOWN_R, 0xAE)         \
	X(VOLUME_UP_R, 0xB0)           \
	X(WWW_HOME_R, 0xB2)            \
	X(KP_SLASH_R, 0xB5)            \
	X(RIGHT_ALT_R, 0xB8)           \
	X(HOME_R, 0xC7)                \
	X(UP_R, 0xC8)                  \
	X(PAGE_UP_R, 0xC9)             \
	X(LEFT_R, 0xCB)                \
	X(RIGHT_R, 0xCD)               \
	X(END_R, 0xCF)                 \
	X(DOWN_R, 0xD0)                \
	X(PAGE_DOWN_R, 0xD1)           \
	X(INSERT_R, 0xD2)              \
	X(DELETE_R, 0xD3)              \
	X(LEFT_GUI_R, 0xDB)            \
	X(RIGHT_GUI_R, 0xDC)           \
	X(APPS_R, 0xDD)                \
	X(POWER_R, 0xDE)               \
	X(SLEEP_R, 0xDF)               \
	X(WAKE_R, 0xE3)                \
	X(WWW_SEARCH_R, 0xE5)          \
	X(WWW_FAVORITES_R, 0xE6)       \
	X(WWW_REFRESH_R, 0xE7)         \
	X(WWW_STOP_R, 0xE8)            \
	X(WWW_FORWARD_R, 0xE9)         \
	X(WWW_BACK_R, 0xEA)            \
	X(MY_COMPUTER_R, 0xEB)         \
	X(EMAIL_R, 0xEC)               \
	X(MEDIA_SELECT_R, 0xED)

// Generate extended scancode enum (using only second byte)
typedef enum {
#define EXT_ENUM_ENTRY(name, code) SCE_##name = code,
	EXTENDED_SCANCODES(EXT_ENUM_ENTRY)
#undef EXT_ENUM_ENTRY
} extended_scancode_t;

// String conversion for extended scancodes
const char* extended_scancode_to_string(extended_scancode_t scancode) {
	switch (scancode) {
#define EXT_STRING_CASE(name, code) \
	case SCE_##name:                \
		return #name;
		EXTENDED_SCANCODES(EXT_STRING_CASE)
#undef EXT_STRING_CASE
	default:
		return "UNKNOWN_EXT";
	}
}

// Validation for extended scancodes
bool validate_extended_scancode(extended_scancode_t scancode) {
	switch (scancode) {
#define EXT_VALID_CASE(name, code) case SCE_##name:
		EXTENDED_SCANCODES(EXT_VALID_CASE)
#undef EXT_VALID_CASE
		return true;
	default:
		return false;
	}
}

#define EXTENED_SCANCODE_LEN (SCE_MEDIA_SELECT + 1)
#define EXTENDED_SCANCODE_RELEASE_OFFSET 0x80
#define EXTENDED_SCANCODE_RELEASE_START 0x90 // 144
