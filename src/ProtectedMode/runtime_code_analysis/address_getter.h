#pragma once

#include "call_real16_wrapper.h"
#include "realmode_functions.h"

/*
 * External symbols from assembly or linker script
 */

extern int stack16_start;
extern int stack16_end;

extern int args16_end;

extern int misc32_s1;
extern int misc32_s2;

extern function_t pm16_to_real16;
extern function_t call_real16_function;
extern function_t resume32;

extern int resume32_end;

/* These two appear to be referenced but not declared in your snippet */
// extern function_t call_real_mode_function_with_argc;
// extern function_t pm32_to_pm16;
// extern function_t add16;

/*
 * Getter API
 */

int* get_stack16_start_address(void);
int* get_stack16_end_address(void);

int* get_args16_start_address(void);
int* get_args16_end_address(void);

int* get_misc32_s1_address(void);
int* get_misc32_s2_address(void);

int* get_resume32_end_address(void);

function_t* get_pm16_to_real16_address(void);
function_t* get_call_real16_function_address(void);
function_t* get_resume32_start_address(void);

function_t* get_call_realmode_func_with_args_address(void);
function_t* get_pm32_to_pm16_address(void);
