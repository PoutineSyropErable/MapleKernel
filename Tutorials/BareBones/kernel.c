#include "virtual_memory.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error \
    "You are not using a cross-compiler, you will most certainly run into trouble"
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

// Static makes it so the symbol name, "vga_entry_color" is in a name space,
// unique to this file, so there is no clash of symbols, if another function
// with the same name is in another file. Static isn't making it not usable in
// other file, that's default behavior. It's for linking. You can see it as
// doing <filename>__vga_entry_color. inline is so there is no call stack, just
// paste that into the assembly itself, useful for small function, where The
// memory increase is worth the dereference and call stack
static inline color_bg_fg vga_entry_color(enum vga_color fg,
                                          enum vga_color bg) {
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

// static const size_t VGA_WIDTH = 80;
// static const size_t VGA_HEIGHT = 25;
// static const size_t VGA_MEM_HEIGHT = 1000; // The maximal size of the
// terminal

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

color_char terminal_big_scrollable_buffer[VGA_WIDTH * VGA_MEM_HEIGHT];

void initialize_terminal(TerminalContext* terminal) {
	terminal->current_write_row = 0;
	terminal->current_write_column = 0;
	terminal->color =
	    vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	const int terminal_vga_text_memory_start = 0xB8000;
	terminal->vga_buffer = (volatile color_char*)terminal_vga_text_memory_start;

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			// 1 2 3 4
			// 5 6 7 8 ---- y *4 + x
			terminal->vga_buffer[index] =
			    vga_entry(' ', terminal->color);
		}
	}

	terminal->big_scrollable_buffer = terminal_big_scrollable_buffer;
	terminal->scroll_row = 0;
	for (size_t y = 0; y < VGA_MEM_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			// 1 2 3 4
			// 5 6 7 8 ---- y *4 + x
			terminal->big_scrollable_buffer[index] =
			    vga_entry(' ', terminal->color);
		}
	}
}

void terminal_update_vga_mem(TerminalContext* terminal) {
	size_t offset = terminal->scroll_row * VGA_WIDTH;

	// need to set the memory value properly now
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			// 1 2 3 4
			// 5 6 7 8 ---- y *4 + x
			terminal->vga_buffer[index] = terminal->big_scrollable_buffer[index + offset];
		}
	}
}

inline void terminal_set_scroll(TerminalContext* terminal, size_t row) {
	size_t max = VGA_MEM_HEIGHT - VGA_HEIGHT;
	if (row > max)
		row = max;
	terminal->scroll_row = row;

	terminal_update_vga_mem(terminal);
}

inline void terminal_scroll_down(TerminalContext* terminal, int scroll_amount) {
	size_t current_scroll = terminal->scroll_row;
	terminal_set_scroll(terminal, current_scroll + scroll_amount);
}

void terminal_setcolor(TerminalContext* term, color_bg_fg color) {
	term->color = color;
}

/*
pos_x: The column number, Right ->,
pos_y: The row number,  Down v
*/
void terminal_putentryat(TerminalContext* term, char c, color_bg_fg color,
                         size_t pos_x, size_t pos_y) {
	const size_t index = pos_y * VGA_WIDTH + pos_x;
	term->big_scrollable_buffer[index] = vga_entry(c, color);

	// test if the entry is currently visible.
	if (pos_y - term->scroll_row >= VGA_HEIGHT) {
		return;
	}
	size_t offset = term->scroll_row * VGA_WIDTH;
	term->vga_buffer[index - offset] = term->big_scrollable_buffer[index];
}

inline void terminal_increase_row(TerminalContext* terminal) {
	terminal->current_write_row++;
	if (terminal->current_write_row - terminal->scroll_row >= VGA_HEIGHT) {
		terminal_scroll_down(terminal, 1);
	}
	if (terminal->current_write_row >= VGA_MEM_HEIGHT) {
		terminal->current_write_row = 0;
		terminal->current_write_column = 0;
		terminal_set_scroll(terminal, 0);
	}
}

void terminal_putchar(TerminalContext* terminal, char c) {
	if (c == '\n') {
		terminal->current_write_column = 0;
		terminal_increase_row(terminal);

		return;
	}
	terminal_putentryat(terminal, c, terminal->color, terminal->current_write_column, terminal->current_write_row);
	terminal->current_write_column++;
	if (terminal->current_write_column == VGA_WIDTH) {
		terminal_increase_row(terminal);
	}
}

void terminal_write(TerminalContext* terminal, const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(terminal, data[i]);
}

void terminal_writestring(TerminalContext* terminal, const char* data) {
	// separate with newlines, if newline character is detected, we need to
	// change the terminal row, and set the colum to 0. why do the above, we put
	// char one at a time
	terminal_write(terminal, data, strlen(data));
}

// Returns the length of the string written.
size_t itoa(int value, char* buffer) {
	size_t pos = 0;
	int negative = 0;

	if (value == 0) {
		buffer[pos++] = '0';
		buffer[pos] = '\0';
		return pos;
	}

	if (value < 0) {
		negative = 1;
		value = -value; // careful: INT_MIN may overflow, but for small kernel code often fine
	}

	// Write digits in reverse
	size_t start = pos;
	while (value > 0) {
		buffer[pos++] = '0' + (value % 10);
		value /= 10;
	}

	if (negative) {
		buffer[pos++] = '-';
	}

	buffer[pos] = '\0';

	// Reverse the string
	size_t end = pos - 1;
	while (start < end) {
		char tmp = buffer[start];
		buffer[start] = buffer[end];
		buffer[end] = tmp;
		start++;
		end--;
	}

	return pos;
}

static inline void wait(unsigned int seconds) {
	volatile unsigned long count;

	// Tuned experimentally for ~1s per unit on a modern CPU
	const unsigned long loops_per_sec = 150000000;

	for (unsigned int s = 0; s < seconds; s++) {
		for (count = 0; count < loops_per_sec; count++) {
			__asm__ volatile("nop");
		}
	}
}

void kernel_main(void) {

	init_paging();
	init_page_bitmap();

	/* Initialize terminal interface */
	TerminalContext term = {0};
	initialize_terminal(&term);

	terminal_set_scroll(&term, 0);
	/* Newline support is left as an exercise. */
	wait(2);
	terminal_writestring(&term, "Hello, kernel World!\n");
	wait(2);
	terminal_writestring(&term, "How are you my friend\n");
	wait(2);
	terminal_writestring(&term, "Test123\n");
	char buf[12];
	for (int i = 0; i < 50; i++) {
		size_t len = itoa(i, buf);
		buf[len] = '\n';     // replace the null terminator with newline
		buf[len + 1] = '\0'; // add new null terminator
		terminal_writestring(&term, buf);
		wait(2);
	}
	terminal_writestring(&term, "This is a nice test\n");
	terminal_writestring(&term, "Last line doesn't need a newline");
}
