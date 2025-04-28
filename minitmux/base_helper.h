#pragma once
#include <stddef.h>

void send_notification(const char* title, const char* text);

typedef struct GapBuffer {
	char* buffer;
	size_t gap_start;
	size_t gap_end;
	size_t capacity;
} GapBuffer;

// Initialize the gap buffer
void gap_buffer_init(GapBuffer* gb, size_t initial_capacity);

void gap_buffer_move_left(GapBuffer* gb);
void gap_buffer_move_right(GapBuffer* gb);

// Insert a character at the cursor
void gap_buffer_insert(GapBuffer* gb, char c);

// Delete a character before the cursor (backspace)
void gap_buffer_delete(GapBuffer* gb);

// Flatten the buffer into a normal null-terminated string
// Caller must free() the result
char* gap_buffer_flatten(GapBuffer* gb);

// Free the gap buffer
void gap_buffer_free(GapBuffer* gb);
