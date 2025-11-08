#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if !defined(__KERNEL__)
#define USER_MODE 1
#else
#define USER_MODE 0
#endif

typedef uint64_t target_uint16_t;

typedef void (*func_ptr_t)(void);

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

typedef struct __attribute__((packed, aligned(4))) {
	uint32_t lower;
	uint32_t higher;
} SegmentDescriptor;

typedef struct __attribute__((packed, aligned(4))) {
	uint16_t limit;
	SegmentDescriptor* base;
} GDT_ROOT;

GDT_ROOT get_gdt_root(void) {

	GDT_ROOT gdt_root;
	__asm__ volatile("sgdt %0" : "=m"(gdt_root));

	return gdt_root;
}

typedef struct __attribute__((packed, aligned(4))) Args16 {
	GDT_ROOT gdt_root;
	uint32_t esp;

	uint16_t ret1;
	uint16_t ret2;

	uint16_t func;
	uint16_t func_cs;

	uint16_t argc;
	uint16_t func_args[12];

} Args16;

Args16 args16_start;

// Internal function: explicit argc
void push_to_args16_with_count(uint16_t argc, ...) {

	bool optional = true;
	if (optional) {
		// This is done later anyway. But might as well for now
		GDT_ROOT gdt_root = get_gdt_root();
		memcpy(&args16_start, &gdt_root, 10);
		args16_start.gdt_root = gdt_root;

		__asm__ volatile(
		    ".intel_syntax noprefix    \n"
		    "mov [args16_start+10], esp\n"
		    ".att_syntax prefix        \n" : : : "memory");
	}

	va_list args;
	va_start(args, argc);

	uint16_t func = va_arg(args, uint32_t);
	args16_start.func = func;
	uint16_t func_cs = va_arg(args, uint32_t);
	args16_start.func_cs = func_cs;

	args16_start.argc = argc - 2;

	for (int i = 0; i < argc; i++) {
		args16_start.func_args[i] = va_arg(args, uint32_t); // read promoted uint32_t
	}

	va_end(args);
}

// Macro wrapper: automatically counts number of arguments
#define push_to_args16(...) \
	push_to_args16_with_count(PP_NARG(__VA_ARGS__), __VA_ARGS__)

void print_args16(const Args16* args) {
	printf("==== Args16 Contents ====\n");

	printf("GDT Root:\n");
	printf("  Limit: 0x%04x\n", args->gdt_root.limit);
	printf("  Base: 0x%p\n", (void*)args->gdt_root.base);

#if !USER_MODE
	if (args->gdt_root.base) {
		printf("  Base Descriptor Lower: 0x%08x\n", args->gdt_root.base->lower);
		printf("  Base Descriptor Higher: 0x%08x\n", args->gdt_root.base->higher);
	}
#endif

	printf("ESP: 0x%08x\n", args->esp);
	printf("Return values: ret1 = %u, ret2 = %u\n", args->ret1, args->ret2);
	printf("Function: 0x%04x, CS: 0x%04x\n", args->func, args->func_cs);
	printf("Argc: %u\n", args->argc);

	printf("Function Arguments:\n");
	for (int i = 0; i < args->argc && i < 12; i++) {
		printf("  arg[%d] = %u\n", i, args->func_args[i]);
	}
	printf("========================\n");
}

void some_func(void) {
	return;
}

struct realmode_address {
	uint16_t func_address;
	uint16_t func_cs;
};

struct realmode_address get_realmode_function_address(void (*func)(void)) {

#if USER_MODE
	uint64_t func_addr_64 = (uint64_t)func;
	uint32_t addr = (uint32_t)func_addr_64;
	addr = addr & 0xFFFFF;
#else
	uint32_t addr = (uint32_t)func;
#endif

	// Real mode can only access 0..0xFFFFF (1 MB)
	if (addr > 0xFFFFF) {
		// Return an invalid CS:IP
		return (struct realmode_address){0xFFFF, 0xFFFF};
	}

	uint16_t func_address = addr & 0xFFFF;
	uint16_t cs = (addr - func_address) >> 16;

	struct realmode_address rm = {.func_address = func_address, .func_cs = cs};
	return rm;
}

int main(void) {
	printf("\n====Start of program======\n\n");

	uint64_t func_address = (uint64_t)some_func;

	uint32_t accessible_address = (uint32_t)func_address;
	printf("%p\n", some_func);
	printf("%zx\n", func_address);

	uint16_t func = 0x1234;
	uint16_t func_cs = 0x6942;

	push_to_args16(func, func_cs, 56, 259, 4269); // argc automatically calculated

	print_args16(&args16_start);

	uint16_t args16_raw[24];
	memcpy(args16_raw, &args16_start, 48);

	printf("Args16 raw: \n");
	for (uint16_t i = 0; i < 24; i += 1) {
		printf("%u = %u\n", i, args16_raw[i]);
	}

	func_ptr_t spoofed = (func_ptr_t)0x0b020;

	struct realmode_address addr = get_realmode_function_address(spoofed);
	printf("addr = {.cs = %x, .base = %x}\n", addr.func_cs, addr.func_address);

	return 0;
}
