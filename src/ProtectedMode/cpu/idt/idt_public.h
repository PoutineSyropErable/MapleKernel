#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	void enable_idt_entry(uint8_t entry_index);
	void disable_idt_entry(uint8_t entry_index);

	bool is_idt_entry_enabled(uint8_t entry_index);

#ifdef __cplusplus
}
#endif
