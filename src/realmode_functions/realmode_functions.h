#include <stdint.h>

__attribute__((optimize("O1"), section(".text.realmode_functions"))) uint16_t add16_ref(uint16_t a, uint16_t b);
__attribute__((optimize("O1"), section(".text.realmode_functions"))) uint16_t ret_5();

__attribute__((optimize("O1"), section(".text.realmode_functions"))) int16_t complex_operation(uint16_t a, uint16_t b, uint16_t c, uint16_t d);
