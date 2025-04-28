#pragma once
#include <stddef.h>

void split_horizontal();
void split_vertical();
void close_current_split();
void nohop();
void set_keybinds();

void play_keybind(char key);

#define ACTION_TABLE_SIZE 256

#define SPLIT_HORIZONTAL_KEY 'h'
#define SPLIT_VERTICAL_KEY 'v'
#define SPLIT_CLOSE_KEY 'x'

typedef void (*ActionFn)(void);
extern ActionFn action_table[ACTION_TABLE_SIZE];
