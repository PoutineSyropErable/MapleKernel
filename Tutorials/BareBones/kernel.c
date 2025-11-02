#include "add16_wrapper.h"
#include "address_getter.c"
#include "os_registers.c"
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

inline void clear_visible_terminal(TerminalContext* terminal) {

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			// 1 2 3 4
			// 5 6 7 8 ---- y *4 + x
			terminal->vga_buffer[index] =
			    vga_entry(' ', terminal->color);
		}
	}
}

void initialize_terminal(TerminalContext* terminal) {
	terminal->current_write_row = 0;
	terminal->current_write_column = 0;
	terminal->color =
	    vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	const int terminal_vga_text_memory_start = 0xB8000;
	terminal->vga_buffer = (volatile color_char*)terminal_vga_text_memory_start;

	clear_visible_terminal(terminal);

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
	if (terminal->current_write_row - terminal->scroll_row > VGA_HEIGHT) {
		terminal_scroll_down(terminal, 1);
	}
	if (terminal->current_write_row >= VGA_MEM_HEIGHT) {
		terminal->current_write_row = 0;
		terminal->current_write_column = 0;
		terminal_set_scroll(terminal, 0);
		clear_visible_terminal(terminal);
	}
}

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t val) {
	/*

	%0 = val : Value to output
	%1 = port: The io port
	*/
	__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void serial_write_char(char c) {
	// Wait until the transmit buffer is empty
	while (!(inb(COM1 + 5) & 0x20))
		;
	outb(COM1, c);
}

void serial_write_string(const char* str) {
	while (*str) {
		serial_write_char(*str++);
	}
}

void terminal_putchar(TerminalContext* terminal, char c) {
	serial_write_char(c);
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

size_t uitoa(uint32_t value, char* buffer) {
	size_t pos = 0;

	if (value == 0) {
		buffer[pos++] = '0';
		buffer[pos] = '\0';
		return pos;
	}

	// Write digits in reverse
	size_t start = pos;
	while (value > 0) {
		buffer[pos++] = '0' + (value % 10);
		value /= 10;
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

size_t int_to_hex(uint32_t value, char* buffer, int uppercase) {
	const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
	int i;

	// Handle zero explicitly
	if (value == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return 1;
	}

	char temp[9]; // Max 8 hex digits + null terminator
	temp[8] = '\0';

	// Convert each nibble
	for (i = 7; i >= 0; i--) {
		uint8_t nibble = value & 0xF;
		temp[i] = digits[nibble];
		value >>= 4;
	}

	// Copy skipping leading zeros
	i = 0;
	while (temp[i] == '0')
		i++;
	int j = 0;
	while (temp[i] != '\0') {
		buffer[j++] = temp[i++];
	}
	buffer[j] = '\0';

	return (size_t)j;
}

static inline void wait(float seconds) {

	volatile unsigned long count;
	const unsigned long loops_per_sec = 150000000UL; // tuned for ~1s per unit

	// total loops = seconds * loops_per_sec
	unsigned long total_loops = (unsigned long)(seconds * loops_per_sec);

	for (unsigned long i = 0; i < total_loops; i++) {
		__asm__ volatile("nop");
	}
}

void print_array_terminal(TerminalContext* term, int* arr, size_t n) {
	char buf[12];
	terminal_writestring(term, "["); // start bracket

	for (size_t i = 0; i < n; i++) {
		itoa(arr[i], buf);
		terminal_writestring(term, buf);

		if (i != n - 1) {
			terminal_writestring(term, ", ");
		}

		// Insert newline every 10 elements to handle scrolling
		if ((i + 1) % 10 == 0) {
			terminal_writestring(term, "\n");
			wait(0.1);
		}
	}

	terminal_writestring(term, "]\n"); // end bracket with newline
}

void print_int_var(TerminalContext* term, int var) {

	char res_buff[12];
	size_t len = itoa(var, res_buff);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(term, res_buff);
}

void print_hex_var(TerminalContext* term, int var) {

	char res_buff[12];
	size_t len = int_to_hex(var, res_buff, 1);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(term, res_buff);
}

void print_uint_var(TerminalContext* term, uint32_t var) {

	char res_buff[12];
	size_t len = uitoa(var, res_buff);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(term, res_buff);
}

void print_something(TerminalContext* term) {

	char buf[12];
	for (int i = 0; i < 10; i++) {
		size_t len = itoa(i, buf);
		buf[len] = '\n';     // replace the null terminator with newline
		buf[len + 1] = '\0'; // add new null terminator
		terminal_writestring(term, buf);
		wait(0.1);
	}
}

int* print_extern_address(TerminalContext* term, char* str, int* func()) {

	terminal_writestring(term, str);
	int* address_value = func();
	print_hex_var(term, (int)address_value);
	terminal_writestring(term, "\n");
	return address_value;
}

uint32_t read32(uint32_t addr) {
	return *(volatile uint32_t*)addr;
}

void terminal_write_uint(TerminalContext* term, char* str, uint32_t val) {

	terminal_writestring(term, str);
	print_uint_var(term, (int)val);
}

void terminal_write_hex(TerminalContext* term, char* str, uint32_t val) {

	terminal_writestring(term, str);
	print_hex_var(term, (int)val);
}

void print_extern_address16(TerminalContext* term, char* str, uint16_t func()) {

	terminal_writestring(term, str);
	uint16_t address_value = func();
	print_int_var(term, (int)address_value);
	// terminal_writestring(term, "\n");
}

void before(TerminalContext* term) {
	terminal_writestring(term, "Before the main execution");
}

void kernel_main(void) {

	// init_paging();
	// init_page_bitmap();

	/* Initialize terminal interface */
	TerminalContext term = {0};
	initialize_terminal(&term);
	terminal_set_scroll(&term, 0);

	terminal_writestring(&term, "\n\n===== Start of Kernel=====\n\n");

	print_extern_address(&term, "The address of stack16_start: ", get_stack16_start_address);
	print_extern_address(&term, "The address of stack16_end: ", get_stack16_end_address);
	print_extern_address(&term, "The address of args16_start: ", get_args16_start_address);
	print_extern_address(&term, "The address of args16_end: ", get_args16_end_address);

	int* add1632_address = print_extern_address(&term, "The address of add1632: ", get_add1632_start_address);
	int* call_add16_address = print_extern_address(&term, "The address of call_add16: ", get_call_add16_address);
	int* add1616_address = print_extern_address(&term, "The address of add1616: ", get_add1616_start_address);
	int* resume32_address = print_extern_address(&term, "The address of resume32: ", get_resume32_start_address);

	// terminal_writestring(&term, "\n");
	// terminal_writestring(&term, "The value of the code at 0xb040: \n");
	// for (int i = 0; i < 50; i++) {
	// 	print_hex_var(&term, add1616_address[i]);
	// }
	//
	// terminal_writestring(&term, "\n");
	// terminal_writestring(&term, "The value of the code at 0xB0A8: \n");
	// for (int i = 0; i < 50; i++) {
	// 	print_hex_var(&term, resume32_address[i]);
	// }

	print_extern_address16(&term, "\nThe value of cs: ", get_cs_selector);
	print_extern_address16(&term, "\nThe value of ss: ", get_ss_selector);
	print_extern_address16(&term, "\nThe value of ds: ", get_ds_selector);
	print_extern_address16(&term, "\nThe value of es: ", get_fs_selector);
	print_extern_address16(&term, "\nThe value of fs: ", get_fs_selector);
	print_extern_address16(&term, "\nThe value of gs: ", get_gs_selector);

	GDT_ROOT gdt_root = get_gdt_root();
	GDT_ENTRY* gdt = (GDT_ENTRY*)gdt_root.base;

	terminal_write_hex(&term, "gdt base address = ", gdt_root.base);
	terminal_write_uint(&term, "gdt size limit = ", gdt_root.limit);

	terminal_writestring(&term, "\nThe gdtr values:\n");
	terminal_write_hex(&term, "gdt[0].low = ", gdt[0].low);
	terminal_write_hex(&term, "gdt[0].high = ", gdt[0].high);
	terminal_write_hex(&term, "gdt[1].low = ", gdt[1].low);
	terminal_write_hex(&term, "gdt[1].high = ", gdt[1].high);
	terminal_write_hex(&term, "gdt[2].low = ", gdt[2].low);
	terminal_write_hex(&term, "gdt[2].high = ", gdt[2].high);
	terminal_write_hex(&term, "gdt[3].low = ", gdt[3].low);
	terminal_write_hex(&term, "gdt[3].high = ", gdt[3].high);
	// print_int_var(&term, gdtr[0]);
	// print_int_var(&term, gdtr[1]);
	// print_int_var(&term, gdtr[2]);

	// wait(25);

	before(&term);
	uint16_t result = 0;
	result = call_add16(25, 56);
	terminal_writestring(&term, "The result of add16: ");
	print_int_var(&term, result);
	wait(25);

	return;

	// int* big_array = (int*)kmalloc(1024 * 1024 * 1024);
	// for (int i = 0; i < 100000; i++) {
	// 	big_array[i] = i;
	// }
	//
	// print_array_terminal(&term, big_array, 100000);
}
