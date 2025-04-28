# Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Output directory for object files
BUILD_DIR = build

# List of source files
SRC = base_helper.c keybinds.c parser.c minitmux.c

# Object files placed into build/
OBJ = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC))

# Final executable name
TARGET = minitmux

# Default target
all: $(BUILD_DIR) $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

# Compile rules with explicit dependencies
$(BUILD_DIR)/base_helper.o: base_helper.c base_helper.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c base_helper.c -o $@

$(BUILD_DIR)/keybinds.o: keybinds.c keybinds.h base_helper.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c keybinds.c -o $@

$(BUILD_DIR)/parser.o: parser.c parser.h base_helper.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c parser.c -o $@

$(BUILD_DIR)/minitmux.o: minitmux.c minitmux.h keybinds.h parser.h base_helper.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c minitmux.c -o $@

# Rule to create build/ directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean

