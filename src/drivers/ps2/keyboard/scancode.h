#pragma once
#include <stddef.h>
#include <stdint.h>

#define SCANCODES(X)       \
	X(ESCAPE, 0x01)        \
	X(1, 0x02)             \
	X(2, 0x03)             \
	X(3, 0x04)             \
	X(4, 0x05)             \
	X(5, 0x06)             \
	X(6, 0x07)             \
	X(7, 0x08)             \
	X(8, 0x09)             \
	X(9, 0x0A)             \
	X(0, 0x0B)             \
	X(MINUS, 0x0C)         \
	X(EQUAL, 0x0D)         \
	X(BACKSPACE, 0x0E)     \
	X(TAB, 0x0F)           \
	X(Q, 0x10)             \
	X(W, 0x11)             \
	X(E, 0x12)             \
	X(R, 0x13)             \
	X(T, 0x14)             \
	X(Y, 0x15)             \
	X(U, 0x16)             \
	X(I, 0x17)             \
	X(O, 0x18)             \
	X(P, 0x19)             \
	X(LBRACKET, 0x1A)      \
	X(RBRACKET, 0x1B)      \
	X(ENTER, 0x1C)         \
	X(LEFT_CTRL, 0x1D)     \
	X(A, 0x1E)             \
	X(S, 0x1F)             \
	X(D, 0x20)             \
	X(F, 0x21)             \
	X(G, 0x22)             \
	X(H, 0x23)             \
	X(J, 0x24)             \
	X(K, 0x25)             \
	X(L, 0x26)             \
	X(SEMICOLON, 0x27)     \
	X(QUOTE, 0x28)         \
	X(BACKTICK, 0x29)      \
	X(LEFT_SHIFT, 0x2A)    \
	X(BACKSLASH, 0x2B)     \
	X(Z, 0x2C)             \
	X(X, 0x2D)             \
	X(C, 0x2E)             \
	X(V, 0x2F)             \
	X(B, 0x30)             \
	X(N, 0x31)             \
	X(M, 0x32)             \
	X(COMMA, 0x33)         \
	X(DOT, 0x34)           \
	X(SLASH, 0x35)         \
	X(RIGHT_SHIFT, 0x36)   \
	X(KP_ASTERISK, 0x37)   \
	X(LEFT_ALT, 0x38)      \
	X(SPACE, 0x39)         \
	X(CAPSLOCK, 0x3A)      \
	X(F1, 0x3B)            \
	X(F2, 0x3C)            \
	X(F3, 0x3D)            \
	X(F4, 0x3E)            \
	X(F5, 0x3F)            \
	X(F6, 0x40)            \
	X(F7, 0x41)            \
	X(F8, 0x42)            \
	X(F9, 0x43)            \
	X(F10, 0x44)           \
	X(NUMLOCK, 0x45)       \
	X(SCROLLLOCK, 0x46)    \
	X(KP_7, 0x47)          \
	X(KP_8, 0x48)          \
	X(KP_9, 0x49)          \
	X(KP_MINUS, 0x4A)      \
	X(KP_4, 0x4B)          \
	X(KP_5, 0x4C)          \
	X(KP_6, 0x4D)          \
	X(KP_PLUS, 0x4E)       \
	X(KP_1, 0x4F)          \
	X(KP_2, 0x50)          \
	X(KP_3, 0x51)          \
	X(KP_0, 0x52)          \
	X(KP_DOT, 0x53)        \
	X(F11, 0x57)           \
	X(F12, 0x58)           \
                           \
	/* Released keys */    \
	X(ESCAPE_R, 0x81)      \
	X(1_R, 0x82)           \
	X(2_R, 0x83)           \
	X(3_R, 0x84)           \
	X(4_R, 0x85)           \
	X(5_R, 0x86)           \
	X(6_R, 0x87)           \
	X(7_R, 0x88)           \
	X(8_R, 0x89)           \
	X(9_R, 0x8A)           \
	X(0_R, 0x8B)           \
	X(MINUS_R, 0x8C)       \
	X(EQUAL_R, 0x8D)       \
	X(BACKSPACE_R, 0x8E)   \
	X(TAB_R, 0x8F)         \
	X(Q_R, 0x90)           \
	X(W_R, 0x91)           \
	X(E_R, 0x92)           \
	X(R_R, 0x93)           \
	X(T_R, 0x94)           \
	X(Y_R, 0x95)           \
	X(U_R, 0x96)           \
	X(I_R, 0x97)           \
	X(O_R, 0x98)           \
	X(P_R, 0x99)           \
	X(LBRACKET_R, 0x9A)    \
	X(RBRACKET_R, 0x9B)    \
	X(ENTER_R, 0x9C)       \
	X(LEFT_CTRL_R, 0x9D)   \
	X(A_R, 0x9E)           \
	X(S_R, 0x9F)           \
	X(D_R, 0xA0)           \
	X(F_R, 0xA1)           \
	X(G_R, 0xA2)           \
	X(H_R, 0xA3)           \
	X(J_R, 0xA4)           \
	X(K_R, 0xA5)           \
	X(L_R, 0xA6)           \
	X(SEMICOLON_R, 0xA7)   \
	X(QUOTE_R, 0xA8)       \
	X(BACKTICK_R, 0xA9)    \
	X(LEFT_SHIFT_R, 0xAA)  \
	X(BACKSLASH_R, 0xAB)   \
	X(Z_R, 0xAC)           \
	X(X_R, 0xAD)           \
	X(C_R, 0xAE)           \
	X(V_R, 0xAF)           \
	X(B_R, 0xB0)           \
	X(N_R, 0xB1)           \
	X(M_R, 0xB2)           \
	X(COMMA_R, 0xB3)       \
	X(DOT_R, 0xB4)         \
	X(SLASH_R, 0xB5)       \
	X(RIGHT_SHIFT_R, 0xB6) \
	X(KP_ASTERISK_R, 0xB7) \
	X(LEFT_ALT_R, 0xB8)    \
	X(SPACE_R, 0xB9)       \
	X(CAPSLOCK_R, 0xBA)    \
	X(F1_R, 0xBB)          \
	X(F2_R, 0xBC)          \
	X(F3_R, 0xBD)          \
	X(F4_R, 0xBE)          \
	X(F5_R, 0xBF)          \
	X(F6_R, 0xC0)          \
	X(F7_R, 0xC1)          \
	X(F8_R, 0xC2)          \
	X(F9_R, 0xC3)          \
	X(F10_R, 0xC4)         \
	X(NUMLOCK_R, 0xC5)     \
	X(SCROLLLOCK_R, 0xC6)  \
	X(KP_7_R, 0xC7)        \
	X(KP_8_R, 0xC8)        \
	X(KP_9_R, 0xC9)        \
	X(KP_MINUS_R, 0xCA)    \
	X(KP_4_R, 0xCB)        \
	X(KP_5_R, 0xCC)        \
	X(KP_6_R, 0xCD)        \
	X(KP_PLUS_R, 0xCE)     \
	X(KP_1_R, 0xCF)        \
	X(KP_2_R, 0xD0)        \
	X(KP_3_R, 0xD1)        \
	X(KP_0_R, 0xD2)        \
	X(KP_DOT_R, 0xD3)      \
	X(F11_R, 0xD7)         \
	X(F12_R, 0xD8)

// 2️⃣ Generate enum
typedef enum {
#define ENUM_ENTRY(name, code) SC_##name = code,
	SCANCODES(ENUM_ENTRY)
#undef ENUM_ENTRY
} scancode_t;

// 3️⃣ Generate string conversion function/macro
#define SCANCODE_TO_STRING_CASE(name, code) \
	case code:                              \
		return #name;

const char* scancode_to_string(scancode_t scancode) {
	switch (scancode) {
#define CASE(name, code) SCANCODE_TO_STRING_CASE(name, code)
		SCANCODES(CASE)
#undef CASE
	default:
		return "UNKNOWN";
	}
}

// 4️⃣ Generate scancode validation function
bool validate_scancode(scancode_t scancode) {
	switch (scancode) {
#define SCANCODE_TO_VALID(name, code) \
	case code:                        \
		return true;
		SCANCODES(SCANCODE_TO_VALID) // ← This line was missing!
#undef SCANCODE_TO_VALID
	default:
		return false;
	}
}

#define SCANCODE_LEN (SC_F12 + 1)
#define SCANCODE_RELEASE_OFFSET 0x80 // 128
#define SCANCODE_RELEASE_START 0x81  // 129
