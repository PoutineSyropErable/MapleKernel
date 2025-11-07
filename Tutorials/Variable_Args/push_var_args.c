#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

typedef uint64_t target_uint16_t;

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

target_uint16_t args16_start[32];

// Internal function: explicit argc
void push_to_args16_with_count(target_uint16_t argc, ...) {
	va_list args;
	va_start(args, argc);

	target_uint16_t function_address = va_arg(args, target_uint16_t); // read promoted int
	args16_start[0] = function_address;
	args16_start[1] = argc;

	for (int i = 0; i < argc; i++) {
		uint16_t val = va_arg(args, int);    // read promoted int
		args16_start[i + 1] = (uint16_t)val; // store as 16-bit
	}

	va_end(args);
}

// Macro wrapper: automatically counts number of arguments
#define push_to_args16(...) \
	push_to_args16_with_count(PP_NARG(__VA_ARGS__), __VA_ARGS__)

int main(void) {
	printf("\n====Start of program======\n\n");

	push_to_args16(56, 259, 4269); // argc automatically calculated

	uint16_t total = args16_start[0]; // argc is stored at args16_start[0]
	for (uint16_t i = 0; i <= total; i++) {
		printf("%u : %zu\n", i, args16_start[i]);
	}

	return 0;
}
