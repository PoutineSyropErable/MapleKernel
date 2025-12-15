// kernel/cpp_main.h
#ifndef CPP_MAIN_H
#define CPP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

// This function will be called from C code
void cpp_kernel_main(void);

#ifdef __cplusplus
}
#endif

#endif // CPP_MAIN_H
