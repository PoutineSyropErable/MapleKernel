// kernel_cpp.h
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// C-visible entry point to transfer control to C++ code
int cpp_main(void);

#ifdef __cplusplus
}
#endif
