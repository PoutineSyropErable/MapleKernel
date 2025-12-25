#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	uint32_t	address;
	const char *name;
} Symbol;

Symbol *load_symbols(const char *filename, uint32_t *count)
{
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

	// Find separator "8\n"
	uint8_t *separator = NULL;
	for (uint32_t i = 0; i < file_size - 1; i++)
	{
		if (data[i] == '8' && data[i + 1] == '\n')
		{
			separator = &data[i];
			break;
		}
	}

	if (!separator)
	{
		free(data);
		return NULL;
	}

	// Calculate number of symbols
	uint32_t addr_section_size = separator - data;
	*count					   = addr_section_size / sizeof(uint32_t);

	// Allocate symbol array
	Symbol *symbols = malloc(*count * sizeof(Symbol));

	// Parse addresses
	uint32_t *addresses = (uint32_t *)data;
	for (uint32_t i = 0; i < *count; i++)
	{
		symbols[i].address = addresses[i];
	}

	// Parse names
	uint8_t *name_ptr = separator + 2; // Skip "8\n"
	for (uint32_t i = 0; i < *count; i++)
	{
		symbols[i].name = (const char *)name_ptr;
		// Move to next null terminator
		while (*name_ptr)
			name_ptr++;
		name_ptr++; // Skip the null
	}

	return symbols;
}

// Example usage:
void main()
{
	uint32_t symbol_count;
	Symbol	*symbols = load_symbols("symbols.bin", &symbol_count);

	if (symbols)
	{
		printf("Loaded %u symbols\n", symbol_count);

		// Find function by address
		for (uint32_t i = 0; i < symbol_count; i++)
		{
			if (symbols[i].address == 0x00205700)
			{ // kernel_main address
				printf("Found kernel_main at 0x%08x\n", symbols[i].address);
				break;
			}
		}

		free(symbols);
	}
}
