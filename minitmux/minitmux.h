// Terminal escape sequences
#include <stdio.h>

#define ESC "\033"

#define TOP_LEFT() printf(ESC "[H"); // move to top left
// Move cursor to row, col (row and col start from 1)
#define GOTO_CURSOR(row, col) printf(ESC "[%d;%dH", (row), (col))

#define MOVE_UP(n) printf(ESC "[%dA", (n))
#define MOVE_DOWN(n) printf(ESC "[%dB", (n))
#define MOVE_RIGHT(n) printf(ESC "[%dC", (n))
#define MOVE_LEFT(n) printf(ESC "[%dD", (n))

#define MOVE(dy, dx)                   \
	do {                               \
		if ((dy) < 0)                  \
			printf(ESC "[%dA", -(dy)); \
		else if ((dy) > 0)             \
			printf(ESC "[%dB", (dy));  \
		if ((dx) < 0)                  \
			printf(ESC "[%dD", -(dx)); \
		else if ((dx) > 0)             \
			printf(ESC "[%dC", (dx));  \
	} while (0)

// Clear the entire screen
#define CLEAR_SCREEN() printf(ESC "[2J")

// Save current cursor position
#define SAVE_CURSOR() printf(ESC "[s")

// Restore cursor position
#define RESTORE_CURSOR() printf(ESC "[u")

// Hide cursor (so it doesn't blink while drawing)
#define HIDE_CURSOR() printf(ESC "[?25l")

// Show cursor (after program exit)
#define SHOW_CURSOR() printf(ESC "[?25h")

#define CTRL_KEY(k) ((k) & 0x1f)

#define ENTER_KEY '\r' // Enter key is ASCII 13
#define ESCAPE_KEY 27  // Escape key
#define BACKSPACE_KEY 127

#define ARROW_UP 'A'
#define ARROW_DOWN 'B'
#define ARROW_RIGHT 'C'
#define ARROW_LEFT 'D'

struct Position {
	int row;
	int col;
};

struct Position get_cursor_position();
#define POSITION_TO_ARRAY(pos) ((int*)(&(pos)))

#define MAX_LINE_LENGTH 200
