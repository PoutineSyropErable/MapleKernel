// com1.h
#ifndef COM1_H
#define COM1_H

#include <stddef.h>
#include <stdint.h>

void	 com1_init(void);
void	 com1_putc(char c);
uint64_t com1_write_c(const char *str);
void	 com1_write_len_c(const char *str, size_t len);

void serial_write_string(const char *str);

#endif
