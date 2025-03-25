#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

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

// Static makes it so the symbol name, "vga_entry_color" is in a name space, unique to this file,
// so there is no clash of symbols, if another function with the same name is in another file.
// Static isn't making it not usable in other file, that's default behavior. It's for linking.
// You can see it as doing <filename>__vga_entry_color.
// inline is so there is no call stack, just paste that into the assembly itself, useful for small function, where
// The memory increase is worth the dereference and call stack
static inline color_bg_fg vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return bg << 4 | fg;

	// 4 bits for the background, 4 for the foreground
	// bg, fg
	// 4, 4
}

typedef uint16_t color_char;

// uc is for unsigned char, from 0 to 255, (The character to display)
static inline color_char vga_entry(unsigned char uc, color_bg_fg color) {
	return (uint16_t)color << 8 | (uint16_t)uc;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
color_bg_fg terminal_color;
color_char* terminal_buffer;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (color_char*)0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			// 1 2 3 4
			// 5 6 7 8 ---- y *4 + x
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(color_bg_fg color) {
	terminal_color = color;
}

void terminal_putentryat(char c, color_bg_fg color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;

		return;
	}
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		//  vv increase terminal row, it's done in an if statement.
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	// separate with newlines, if newline character is detected, we need to change the terminal row, and set the colum to 0.
	// why do the above, we put char one at a time
	terminal_write(data, strlen(data));
}

void kernel_main(void) {
	/* Initialize terminal interface */
	terminal_initialize();

	/* Newline support is left as an exercise. */
	terminal_writestring("Hello, kernel World!\n");
	terminal_writestring("How are you my friend\n");
}
