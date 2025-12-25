#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t address;
    const char *name;
} Symbol;

uint8_t *load_file(const char *filename, size_t *size) {
    FILE *file = fopen(filename, "rb");
    if (!file)
	return NULL;

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read entire file
    uint8_t *data = malloc(file_size);
    fread(data, 1, file_size, file);
    fclose(file);
    *size = file_size;

    return data;
}

Symbol *parse_mem(uint8_t *data, size_t file_size, uint32_t *count) {
    // Find separator "8\n"
    uint8_t *separator = NULL;
    for (uint32_t i = 0; i < file_size - 1; i++) {
	if (data[i] == '8' && data[i + 1] == '\n') {
	    separator = &data[i];
	    printf("Found the separator at %p\n", separator);
	    break;
	}
    }

    if (!separator) {
	free(data);
	return NULL;
    }

    // Calculate number of symbols
    uint32_t addr_section_size = separator - data;
    *count = addr_section_size / sizeof(uint32_t);

    // Allocate symbol array
    Symbol *symbols = malloc(*count * sizeof(Symbol));

    // Parse addresses
    uint32_t *addresses = (uint32_t *)data;
    for (uint32_t i = 0; i < *count; i++) {
	symbols[i].address = addresses[i];
	// printf("The address is %u\n", addresses[i]);
    }

    // Parse names
    uint8_t *name_ptr = separator + 2; // Skip "8\n"
    for (uint32_t i = 0; i < *count; i++) {
	symbols[i].name = (const char *)name_ptr;
	// Move to next null terminator
	while (*name_ptr)
	    name_ptr++;
	name_ptr++; // Skip the null
    }

    return symbols;
}

// Example usage:
int main() {
    size_t size;
    uint8_t *file_content = load_file("symbols.bin", &size);

    uint32_t symbol_count;
    Symbol *symbols = parse_mem(file_content, size, &symbol_count);
    if (!symbols) {
	return 1;
    }

    printf("Loaded %u symbols\n", symbol_count);

    char search_symbol[] = "kernel_main";

    // Find function by address
    for (uint32_t i = 0; i < symbol_count; i++) {
	// printf("%s, %u\n", symbols[i].name, symbols[i].address);
	if (!strcmp(symbols[i].name, search_symbol)) { // kernel_main address
	    printf("Found kernel_main at 0x%08x\n", symbols[i].address);
	    break;
	}
    }

    free(symbols);
}
