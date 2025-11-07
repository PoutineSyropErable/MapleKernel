#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

// Define a typedef for terminal colors based on uint8_t
typedef uint8_t color_bg_fg;
typedef uint16_t color_char;

#define VGA_MMIO_BASE 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEM_HEIGHT 1000

typedef struct {
	size_t current_write_row;
	size_t current_write_column;
	size_t scroll_row;
	color_bg_fg color;
	volatile color_char* vga_buffer;   // The actual visible data
	color_char* big_scrollable_buffer; // The memory. Doesn't need to be volatile
} TerminalContext;

inline void clear_visible_terminal(TerminalContext* terminal);

void initialize_terminal(TerminalContext* terminal);

void terminal_update_vga_mem(TerminalContext* terminal);

void terminal_set_scroll(TerminalContext* terminal, size_t row);
inline void terminal_scroll_down(TerminalContext* terminal, int scroll_amount);

void terminal_setcolor(TerminalContext* term, color_bg_fg color);

void terminal_putentryat(TerminalContext* term, char c, color_bg_fg color, size_t pos_x, size_t pos_y);

inline void terminal_increase_row(TerminalContext* terminal);

/* ======= Serial std out ======== */

#define COM1 0x3F8

void serial_write_char(char c);

void serial_write_string(const char* str);

void terminal_putchar(TerminalContext* terminal, char c);

void terminal_write(TerminalContext* terminal, const char* data, size_t size);

void terminal_writestring(TerminalContext* terminal, const char* data);

/* === ===   */

extern TerminalContext term;
