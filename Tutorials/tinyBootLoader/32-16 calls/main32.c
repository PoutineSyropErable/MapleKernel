#include <stdint.h>
#include <stdio.h>

extern uint16_t add16(uint16_t a, uint16_t b);

// Kernel-style wrapper that "switches" to 16-bit
uint16_t add_wrapper(uint32_t a, uint32_t b) {
	uint16_t a16 = a & 0xFFFF;
	uint16_t b16 = b & 0xFFFF;
	uint16_t result;

	__asm__ volatile(
	    "cli\n" // disable interrupts
	    // --- save 32-bit registers ---
	    "push eax\n"
	    "push ebx\n"
	    "push ecx\n"
	    "push edx\n"
	    "push esi\n"
	    "push edi\n"

	    // --- here in real kernel mode you would switch to 16-bit ---
	    // simulate calling 16-bit function
	    "mov ax, %[arg1]\n"
	    "mov bx, %[arg2]\n"
	    "call add16\n"
	    "mov %[res], ax\n"

	    // --- restore 32-bit registers ---
	    "pop edi\n"
	    "pop esi\n"
	    "pop edx\n"
	    "pop ecx\n"
	    "pop ebx\n"
	    "pop eax\n"
	    "sti\n" // enable interrupts
	    : [res] "=r"(result)
	    : [arg1] "r"(a16), [arg2] "r"(b16)
	    : "ax", "bx");

	return result;
}

int main() {
	uint16_t result;

	result = add_wrapper(40000, 30000); // addition wraps around 16-bit

	printf("Result: %u\n", result); // unsigned print
	return 0;
}
