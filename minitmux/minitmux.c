#include "minitmux.h"
#include "base_helper.h"
#include "keybinds.h"
#include "parser.h"
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;
struct Position term_size;
// Terminal input/Output Structures terminal ios

// --- Terminal control functions ---

void disable_raw_mode() {
	SHOW_CURSOR();
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	// save the attribute of the terminal associated with std input (keyboard input) to orgin_termios.
	atexit(disable_raw_mode); // Ensure cleanup on exit

	struct termios raw = orig_termios;

	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	raw.c_cflag |= (CS8);
	raw.c_oflag &= ~(OPOST);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

	HIDE_CURSOR();
}

struct Position get_terminal_size() {
	struct winsize w;
	struct Position p = {0, 0};

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
		perror("ioctl");
		exit(1);
	}

	p.row = w.ws_row;
	p.col = w.ws_col;
	return p;
}

void refresh_line(GapBuffer* gb) {
	// 1. Go back to start of line
	printf("\r");

	// 2. Clear the entire line from cursor
	printf("\033[K");

	// 3. Optional: Indent if you want (ex: tab)
	printf("\t");

	// 4. Flatten the buffer and print it
	char* flattened = gap_buffer_flatten(gb);
	printf("%s", flattened);
	free(flattened);

	// 5. Move back to start and move to cursor position
	printf("\r\t"); // Return + indent

	for (size_t i = 0; i < gb->gap_start; ++i) {
		MOVE_RIGHT(1);
	}

	fflush(stdout);
}

// --- Main loop ---

void main_loop() {
	bool keybindMode = false;
	bool cursorVisible = false;
	bool firstTime = true;

	char currentLine[MAX_LINE_LENGTH + 1] = {'\0'};
	GapBuffer currentLineGB;
	gap_buffer_init(&currentLineGB, MAX_LINE_LENGTH + 1);

	char c_to_write = '\0';

	while (true) {
		char c;
		ssize_t n = read(STDIN_FILENO, &c, 1);
		// Blocks, so event driven ^^
		if (n == -1) {
			perror("read");
			exit(1);
		}

		char message[100];
		snprintf(message, sizeof(message), "Pressed key: %d (char: %c)\n", c, (c >= 32 && c <= 126) ? c : '.');
		send_notification("key press", message);

		switch (c) {
		case CTRL_KEY('q'):

			goto endofloop;
			break;

		case ENTER_KEY:
			printf("\r\n\t");
			fflush(stdout);

			disable_raw_mode();
			char* line = gap_buffer_flatten(&currentLineGB);
			parseCurrentLine(line);
			free(line);
			gap_buffer_free(&currentLineGB);
			gap_buffer_init(&currentLineGB, MAX_LINE_LENGTH + 1);

			enable_raw_mode();
			break;

		case ARROW_LEFT:
			gap_buffer_move_left(&currentLineGB);
			break;

		case ARROW_RIGHT:
			gap_buffer_move_right(&currentLineGB);
			break;

		case BACKSPACE_KEY:
			send_notification("backswapce", "pressed backsapce");
			gap_buffer_delete(&currentLineGB);
			refresh_line(&currentLineGB);

			break;

		case CTRL_KEY('c'):
			cursorVisible = !cursorVisible;
			if (cursorVisible) {
				SHOW_CURSOR();

			} else {
				HIDE_CURSOR();
			}
			break;

		case CTRL_KEY('b'):
			keybindMode = true;
			send_notification("key press", "Pressed C-b");
			break;

		default:
			if (keybindMode) {
				play_keybind(c);
				keybindMode = false;

			} else {
				gap_buffer_insert(&currentLineGB, c);

				c_to_write = c;
			}

			break;
		}
		// end of switch

		if (firstTime) {

			send_notification("clear", "clearing whole screen");

			CLEAR_SCREEN();
			TOP_LEFT();
			firstTime = false;
		}

		if (c_to_write) {
			putchar(c_to_write);
		}

		fflush(stdout);
	}
endofloop:;
}

// --- Entry point ---
void handle_sigwinch(int unused) {
	term_size = get_terminal_size();
	// Redraw your layout here based on new size
}

int main() {

	set_keybinds();

	term_size.row = -1;
	term_size.col = -1;
	term_size = get_terminal_size();
	signal(SIGWINCH, handle_sigwinch);

	enable_raw_mode();

	CLEAR_SCREEN();
	GOTO_CURSOR(1, 1);
	TOP_LEFT();
	// it starts at 1,1 not 0,0

	printf("Mini-tmux: Press Ctrl+Q to quit.\n");
	fflush(stdout);

	main_loop();

	return 0;
}
