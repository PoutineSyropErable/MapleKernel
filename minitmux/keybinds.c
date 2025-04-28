#include "keybinds.h"
#include "base_helper.h"
#include <stddef.h>

void split_horizontal() {
	send_notification("splits", "horizontal");
}
void split_vertical() {
	send_notification("splits", "horizontal");
}
void close_current_split() {
	send_notification("splits", "close current");
}

void nohop() {
}
ActionFn action_table[ACTION_TABLE_SIZE] = {NULL};

void set_keybinds() {

	for (size_t function_index; function_index < ACTION_TABLE_SIZE; function_index++) {
		action_table[function_index] = nohop;
	}

	action_table[SPLIT_HORIZONTAL_KEY] = split_horizontal;
	action_table[SPLIT_VERTICAL_KEY] = split_vertical;
	action_table[SPLIT_CLOSE_KEY] = close_current_split;
}

void play_keybind(char key) {
	action_table[key]();
}
