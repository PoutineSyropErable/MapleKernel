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

// Option 1: typedef for a 2D VGA buffer
typedef color_char vga_buffer_t[VGA_HEIGHT][VGA_WIDTH];
typedef color_char big_vga_buffer_t[VGA_MEM_HEIGHT][VGA_WIDTH];

typedef struct {
	size_t current_write_row;
	size_t current_write_column;
	size_t scroll_row;
	color_bg_fg color;
	volatile color_char* vga_buffer; // The actual visible data
	// could use a type where it's
	big_vga_buffer_t big_scrollable_buffer; // The memory. Doesn't need to be volatile
} TerminalContext;

void initialize_terminal();

void terminal_update_vga_mem();

void terminal_set_scroll(size_t row);

void terminal_setcolor(color_bg_fg color);

void terminal_putentryat(char c, color_bg_fg color, size_t pos_x, size_t pos_y);

inline void clear_visible_terminal();
inline void terminal_scroll_down(int scroll_amount);
inline void terminal_increase_row();

// extern inline void clear_visible_terminal();
// extern inline void terminal_scroll_down(int scroll_amount);
// extern inline void terminal_increase_row();

/* ======= Serial std out ======== */

#define COM1 0x3F8

void serial_write_char(char c);

void serial_write_string(const char* str);

void terminal_putchar(char c);

void terminal_write(const char* data, size_t size);

void terminal_writestring(const char* data);

/* === Global values ===   */

extern TerminalContext term;

/* ===== Print helpers ====== */

void print_array_terminal(int* arr, size_t n);

void print_int_var(int var);
void print_uint_var(uint32_t var);
void print_uint_var_no_newline(uint32_t var);

void print_hex_var(uint32_t var);

void print_hex_ptr(void* ptr);

void print_something(void);

int* print_extern_address(char* str, int* func(void));

void terminal_write_uint(char* str, uint32_t val);
void terminal_write_uint_no_newline(char* str, uint32_t val);

void terminal_write_hex(char* str, uint32_t val);

void terminal_write_ptr(char* str, void* val);

void print_extern_address16(char* str, uint16_t func(void));

void terminal_write_uint16_array_newlines(const uint16_t* arr, char* array_name, size_t n);
