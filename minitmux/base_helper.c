#include "base_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void send_notification(const char* title, const char* text) {
	char command[512];

	// Build the notify-send command safely
	snprintf(command, sizeof(command),
	         "notify-send -t 1000 \"%s\" \"%s\"", title, text);

	// Execute the command
	int result = system(command);

	if (result == -1) {
		perror("system");
	}
}

void safe_free(void** ptr) {
	if (ptr && *ptr) {
		free(*ptr);
		*ptr = NULL;
	}
}

// Initialize the gap buffer
void gap_buffer_init(GapBuffer* gb, size_t initial_capacity) {
	gb->capacity = initial_capacity;
	gb->buffer = malloc(gb->capacity);
	if (!gb->buffer) {
		perror("malloc");
		exit(1);
	}
	memset(gb->buffer, 0, gb->capacity);
	gb->gap_start = 0;
	gb->gap_end = gb->capacity;

	gb->flattened = NULL;
}

void gap_buffer_set_flatten(GapBuffer* gb) {

	if (gb->flattened) {
		free(gb->flattened);
		gb->flattened = NULL;
	}
	gb->flattened = gap_buffer_flatten(gb);
}

// Move cursor one position left
void gap_buffer_move_left(GapBuffer* gb) {
	if (gb->gap_start > 0) {
		gb->gap_start--;
		gb->buffer[gb->gap_end - 1] = gb->buffer[gb->gap_start];
		gb->gap_end--;
	}
}

// Move cursor one position right
void gap_buffer_move_right(GapBuffer* gb) {
	if (gb->gap_end < gb->capacity) {
		gb->buffer[gb->gap_start] = gb->buffer[gb->gap_end];
		gb->gap_start++;
		gb->gap_end++;
	}
}

// Insert a character at the cursor position
void gap_buffer_insert(GapBuffer* gb, char c) {
	if (gb->gap_start == gb->gap_end) {
		// Gap is full, need to grow
		size_t new_capacity = gb->capacity * 2;
		char* new_buffer = malloc(new_capacity);
		if (!new_buffer) {
			perror("malloc");
			exit(1);
		}
		// Copy parts before and after gap
		memcpy(new_buffer, gb->buffer, gb->gap_start);
		size_t after_gap = gb->capacity - gb->gap_end;
		memcpy(new_buffer + new_capacity - after_gap, gb->buffer + gb->gap_end, after_gap);

		gb->gap_end = new_capacity - after_gap;
		gb->capacity = new_capacity;

		free(gb->buffer);
		gb->buffer = new_buffer;
	}

	gb->buffer[gb->gap_start++] = c;

	gap_buffer_set_flatten(gb);
}

// Delete a character before the cursor (backspace)
void gap_buffer_delete(GapBuffer* gb) {
	if (gb->gap_start > 0) {
		gb->gap_start--;
	}

	gap_buffer_set_flatten(gb);
}

// Flatten the gap buffer into a null-terminated string
char* gap_buffer_flatten(GapBuffer* gb) {
	size_t size_before_gap = gb->gap_start;
	size_t size_after_gap = gb->capacity - gb->gap_end;
	size_t total_size = size_before_gap + size_after_gap;

	char* result = malloc(total_size + 1); // +1 for null terminator
	if (!result) {
		perror("malloc");
		exit(1);
	}

	memcpy(result, gb->buffer, size_before_gap);
	memcpy(result + size_before_gap, gb->buffer + gb->gap_end, size_after_gap);
	result[total_size] = '\0';

	return result;
}

// Clean up memory
void gap_buffer_free(GapBuffer* gb) {
	free(gb->buffer);
	gb->buffer = NULL;
	gb->capacity = 0;
	gb->gap_start = gb->gap_end = 0;

	if (gb->flattened) {
		free(gb->flattened);
	}
	gb->flattened = NULL;
}
