#pragma once
#include "f3_segment_descriptor_internals.h"
#include "more_types.h"
#include "ppnargs.h"
#include <stddef.h>
#include <stdint.h>

typedef SegmentDescriptor GDT_ENTRY;

typedef struct __attribute__((packed, aligned(4))) {
	uint32_t lower;
	uint32_t higher;
} SegmentDescriptor1; // just so it's easily present here now

typedef struct __attribute__((packed, aligned(4))) {
	uint16_t limit;
	GDT_ENTRY* base;
} GDT_ROOT;

extern GDT_ROOT GDT16_DESCRIPTOR;
typedef int*(address_getter_function_t)(void); // a function pointer

typedef struct __attribute__((packed)) Args16 {
	GDT_ROOT gdt_root;
	// uint16_t pad; // (padded due to esp wanting to)
	uint32_t esp;
	uint16_t ss;
	uint16_t cs;

	uint16_t ret1;
	uint16_t ret2;

	uint16_t func;
	uint16_t func_cs;

	uint16_t argc;
	uint16_t func_args[11];

} Args16;

extern Args16 args16_start;

GDT_ROOT get_gdt_root(void);

void print_args16(const Args16* args);

int print_args16_more(void);

uint16_t call_real_mode_function_with_argc(uint32_t argc, ...);

// Macro wrapper: automatically counts number of arguments
#define call_real_mode_function(...) \
	call_real_mode_function_with_argc(PP_NARG(__VA_ARGS__), __VA_ARGS__)

struct realmode_address {
	uint16_t func_address;
	uint16_t func_cs;
};

extern uint16_t pm32_to_pm16();
