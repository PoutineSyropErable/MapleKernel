#pragma once
#include "pit.h"
#include "static_assert.h"
#include <stddef.h>
#include <stdint.h>

namespace pit
{

// =============== End
void send_wait_count_command(uint32_t freq_divider_count);
void start_loop_wait(uint32_t freq_divider_count);
int	 wait(float seconds);

// new_timeout = false must only be used when there is just one core that uses this.
// Anyway, pit timeout are dangerous.
int short_timeout(float seconds, volatile uint32_t *finished, bool new_timeout);

} // namespace pit
