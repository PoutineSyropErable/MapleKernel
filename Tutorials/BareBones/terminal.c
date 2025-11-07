
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
