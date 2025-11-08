#pragma once
#include "f3_segment_descriptor_internals.h"
#include <stddef.h>
#include <stdint.h>

// Preprocessor macro to count arguments
#define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(                                     \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,          \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
    _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
    _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
    N, ...) N
#define PP_RSEQ_N()                             \
	50, 49, 48, 47, 46, 45, 44, 43, 42, 41,     \
	    40, 39, 38, 37, 36, 35, 34, 33, 32, 31, \
	    30, 29, 28, 27, 26, 25, 24, 23, 22, 21, \
	    20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
	    10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

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
typedef void (*func_ptr_t)(void); // a function pointer

typedef struct __attribute__((packed)) Args16 {
	GDT_ROOT gdt_root;
	// uint16_t pad; // (padded due to esp wanting to)
	uint32_t esp;

	uint16_t ret1;
	uint16_t ret2;

	uint16_t func;
	uint16_t func_cs;

	uint16_t argc;
	uint16_t func_args[12];

} Args16;

extern Args16 args16_start;

GDT_ROOT get_gdt_root(void);

void print_args16(const Args16* args);

int print_args16_more(void);

void push_to_args16_with_count(uint32_t argc, ...);

// Macro wrapper: automatically counts number of arguments
#define push_to_args16(...) \
	push_to_args16_with_count(PP_NARG(__VA_ARGS__), __VA_ARGS__)

struct realmode_address {
	uint16_t func_address;
	uint16_t func_cs;
};
