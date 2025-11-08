#include <stdint.h>

__attribute__((naked, section(".text_realmode_functions"))) uint16_t add16(uint16_t a, uint16_t b);

__attribute__((section(".text_realmode_functions"))) uint16_t add16_ref(uint16_t a, uint16_t b);

__attribute__((section(".text_realmode_functions"))) int16_t complex_operation(uint16_t a, uint16_t b, uint16_t c, uint16_t d);
