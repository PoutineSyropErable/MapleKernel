// src/Main.cpp
module; // Global module fragment

#include "main_cpp.h"
#include "stdio.h" // For kprintf

export module MainBridge; // Optional: export a module if you want

import A;
import B;
import C;
import D;

extern "C" int module_main_cpp() {
	kprintf("Start of greet A, %u\n", 1);
	greetA();
	kprintf("\nStart of greet B, %f\n", 1.5);
	greetB();
	kprintf("\nStart of greet C, %d\n", -1);
	greetC();
	kprintf("\nStart of greet D, %f\n", -2.65);
	greetD();
	[[maybe_unused]] int x = FUNNY_NUMBER;

	int c = lolc();
	float d = lol_d();
	kprintf("\n C result: %d\n", c);
	// kprintf("\n D result: %f\n", d);
	return 0;
}
