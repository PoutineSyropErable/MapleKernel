#include <stdint.h>

void delay() {
    for (volatile uint64_t i = 0; i < 100000000; ++i);
}

void shutdown() {
    uint16_t port = 0x604;
    uint16_t val = 0x2000;
    __asm__ __volatile__("outw %0, %1" : : "a"(val), "Nd"(port));
}

void kernel_main() {
    const char* msg = "Hello from x86_64 kernel!";
    char* vga = (char*)0xB8000;
    for (int i = 0; msg[i]; ++i) {
        vga[i * 2] = msg[i];
        vga[i * 2 + 1] = 0x0F;
    }

    delay();
    shutdown();

    while (1) __asm__ __volatile__("hlt");
}

