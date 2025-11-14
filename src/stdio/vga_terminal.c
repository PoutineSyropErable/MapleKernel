#include "pit_timer.h"
#include "string_helper.h"
#include "vga_terminal.h"

// Static makes it so the symbol name, "vga_entry_color" is in a name space,
// unique to this file, so there is no clash of symbols, if another function
// with the same name is in another file. Static isn't making it not usable in
// other file, that's default behavior. It's for linking. You can see it as
// doing <filename>__vga_entry_color. inline is so there is no call stack, just
// paste that into the assembly itself, useful for small function, where The
// memory increase is worth the dereference and call stack
static inline color_bg_fg vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return bg << 4 | fg;

	// 4 bits for the background, 4 for the foreground
	// bg, fg
	// 4, 4
}

// uc is for unsigned char, from 0 to 255, (The character to display)
static inline color_char vga_entry(unsigned char uc, color_bg_fg color) {
	return (uint16_t)color << 8 | (uint16_t)uc;
}

// color_char terminal_big_scrollable_buffer[VGA_WIDTH * VGA_MEM_HEIGHT];

TerminalContext term = {0};

static inline void set_vga_element(uint8_t pos_x, uint8_t pos_y, color_char c) {
	(*term.vga_buffer)[pos_y][pos_x] = c;
}

void clear_visible_terminal() {

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			// 1 2 3 4
			// 5 6 7 8 ---- y *4 + x
			set_vga_element(x, y, vga_entry_color(' ', term.color));
		}
	}
}

void initialize_terminal() {

	term.current_write_row = 0;
	term.current_write_column = 0;
	term.color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	term.vga_buffer = (volatile vga_buffer_t*)VGA_MMIO_BASE;

	clear_visible_terminal();

	term.scroll_row = 0;
	for (size_t y = 0; y < VGA_MEM_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			// 1 2 3 4
			// 5 6 7 8 ---- y *4 + x
			term.big_scrollable_buffer[y][x] = vga_entry(' ', term.color);
		}
	}
}

void terminal_update_vga_mem() {
	// need to set the memory value properly now
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			// 1 2 3 4
			// 5 6 7 8 ---- y *4 + x
			// y goes down
			// term.vga_buffer[index] = term.big_scrollable_buffer[index + offset];
			set_vga_element(x, y, term.big_scrollable_buffer[y + term.scroll_row][x]);
			// (y0)*VGA_WIDTH + x0 + (sr*VGA_WIDTH)
			// (y0 + sr)*VGA_WIDTH + x0
		}
	}
}

void terminal_set_scroll(size_t row) {

	size_t max = VGA_MEM_HEIGHT - VGA_HEIGHT;
	if (row > max)
		row = max;
	term.scroll_row = row;

	terminal_update_vga_mem();
}

void terminal_scroll_down(int scroll_amount) {

	size_t current_scroll = term.scroll_row;
	terminal_set_scroll(current_scroll + scroll_amount);
}

void terminal_setcolor(color_bg_fg color) {

	term.color = color;
}

/*
pos_x: The column number, Right ->,
pos_y: The row number,  Down v
*/
void terminal_putentryat(char c, color_bg_fg color, size_t pos_x, size_t pos_y) {
	const size_t index = pos_y * VGA_WIDTH + pos_x;
	term.big_scrollable_buffer[pos_y][pos_x] = vga_entry(c, color);

	// test if the entry is currently visible.
	if (pos_y - term.scroll_row >= VGA_HEIGHT) {
		return;
	}
	size_t offset = term.scroll_row * VGA_WIDTH;
	color_char colored_c = term.big_scrollable_buffer[pos_y - term.scroll_row][pos_x];
	set_vga_element(pos_x, pos_y - term.scroll_row, colored_c);
}

void terminal_increase_row() {

	term.current_write_column = 0;
	// should it do it here? or next to the caller? Can a row increase without changing the columns work?

	term.current_write_row++;
	if (term.current_write_row - term.scroll_row > VGA_HEIGHT) {
		terminal_scroll_down(1);
	}
	if (term.current_write_row >= VGA_MEM_HEIGHT) {
		term.current_write_row = 0;
		term.current_write_column = 0;
		terminal_set_scroll(0);
		clear_visible_terminal();
	}
}

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

void terminal_putchar(char c) {

	// should be redundant and not happen
	if (c == '\0') {
		return;
	}
	serial_write_char(c);

	// serial_write_char(c);
	if (c == '\n') {
		term.current_write_column = 0;
		terminal_increase_row();

		return;
	}
	if (c == '\t') {
		uint8_t spaceToAdd = 4;
		uint8_t diff = VGA_WIDTH - term.current_write_column;
		if (4 > diff) {
			spaceToAdd = diff;
		}
		for (uint8_t i = 0; i < spaceToAdd; i++) {
			terminal_putchar(' ');
		}
		return;
	}

	terminal_putentryat(c, term.color, term.current_write_column, term.current_write_row);
	term.current_write_column++;
	if (term.current_write_column == VGA_WIDTH) {
		term.current_write_column = 0;
		terminal_increase_row();
	}
}

void terminal_write(const char* data, size_t size) {

	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	// separate with newlines, if newline character is detected, we need to
	// change the terminal row, and set the colum to 0. why do the above, we put
	// char one at a time
	terminal_write(data, strlen(data));
}

/*  ==== The helpers and nice ======  */

void print_array_terminal(int* arr, size_t n) {
	char buf[12];
	terminal_writestring("["); // start bracket

	for (size_t i = 0; i < n; i++) {
		itoa(arr[i], buf);
		terminal_writestring(buf);

		if (i != n - 1) {
			terminal_writestring(", ");
		}

		// Insert newline every 10 elements to handle scrolling
		if ((i + 1) % 10 == 0) {
			terminal_writestring("\n");
			wait(0.1);
		}
	}

	terminal_writestring("]\n"); // end bracket with newline
}

void terminal_write_uint16_array(uint16_t* arr, char* array_name, size_t n) {
	char buf[12];
	terminal_writestring(array_name);
	terminal_writestring(" = ["); // start bracket

	for (size_t i = 0; i < n; i++) {
		itoa(arr[i], buf);
		terminal_writestring(buf);

		if (i != n - 1) {
			terminal_writestring(", ");
		}

		// Insert newline every 10 elements to handle scrolling
		if ((i + 1) % 10 == 0) {
			terminal_writestring("\n");
			wait(0.1);
		}
	}

	terminal_writestring("]\n"); // end bracket with newline
}

void terminal_write_uint16_array_newlines(const uint16_t* arr, char* array_name, size_t n) {
	for (uint32_t i = 0; i < n; i++) {
		terminal_writestring(array_name);
		terminal_write_uint_no_newline("[", i);
		terminal_write_uint("] = ", arr[i]);
	}
}

void print_int_var(int var) {

	char res_buff[12];
	size_t len = itoa(var, res_buff);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(res_buff);
}

void print_int_var_no_newline(int var) {

	char res_buff[12];
	size_t len = itoa(var, res_buff);
	res_buff[len] = '\0'; // add new null terminator
	terminal_writestring(res_buff);
}

void print_hex_var(uint32_t var) {

	char res_buff[12];
	size_t len = uint_to_hex(var, res_buff, 1);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(res_buff);
}

void print_hex_var_no_newline(uint32_t var) {

	char res_buff[12];
	size_t len = uint_to_hex(var, res_buff, 1);
	res_buff[len] = '\0'; // add new null terminator
	terminal_writestring(res_buff);
}

void print_hex_ptr(void* ptr) {

	char res_buff[12];
	size_t len = ptr_to_hex(ptr, res_buff, 1);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(res_buff);
}

void print_uint_var(uint32_t var) {

	char res_buff[12];
	size_t len = uitoa(var, res_buff);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(res_buff);
}

void print_uint_var_no_newline(uint32_t var) {

	char res_buff[20];
	size_t len = uitoa(var, res_buff);
	res_buff[len] = '\0'; // replace the null terminator with newline

	terminal_writestring(res_buff);
}

void print_float_var_no_newline(float var) {

	char res_buff[20];
	size_t len = ftoa(var, res_buff, 6);
	res_buff[len] = '\0'; // replace the null terminator with newline

	terminal_writestring(res_buff);
}

static inline uint8_t min(uint8_t a, uint8_t b) {
	if (a < b) {
		return a;
	}
	return b;
}

void print_float_var_no_newline_precision(float var, uint8_t precision) {

	const uint8_t max_precision = 6;
	precision = min(precision, max_precision);
	const uint8_t max_float_exponent = 38;
	const uint8_t safety_and_dot_char = 3 + 1;

	const uint8_t bounded_size = max_float_exponent + safety_and_dot_char + max_precision;
	char res_buff[bounded_size];
	size_t len = ftoa(var, res_buff, precision);
	res_buff[len] = '\0'; // replace the null terminator with newline

	terminal_writestring(res_buff);
}

void print_something() {

	char buf[12];
	for (int i = 0; i < 10; i++) {
		size_t len = itoa(i, buf);
		buf[len] = '\n';     // replace the null terminator with newline
		buf[len + 1] = '\0'; // add new null terminator
		terminal_writestring(buf);
		wait(0.1);
	}
}

int* print_extern_address(char* str, int* func()) {

	terminal_writestring(str);
	int* address_value = func();
	print_hex_ptr((void*)address_value);
	terminal_writestring("\n");
	return address_value;
}

void terminal_write_uint(char* str, uint32_t val) {

	terminal_writestring(str);
	print_uint_var(val);
}

void terminal_write_uint_no_newline(char* str, uint32_t val) {

	terminal_writestring(str);
	print_uint_var_no_newline(val);
}

void terminal_write_hex(char* str, uint32_t val) {

	terminal_writestring(str);
	print_hex_var(val);
}

void terminal_write_ptr(char* str, void* val) {

	terminal_writestring(str);
	print_hex_ptr(val);
}

void print_extern_address16(char* str, uint16_t func()) {

	terminal_writestring(str);
	uint16_t address_value = func();
	print_int_var((int)address_value);
	// terminal_writestring( "\n");
}
