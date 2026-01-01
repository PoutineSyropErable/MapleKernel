#include "apic.hpp"
#include "apic_timer_interrupt_handler.hpp"
#include "apic_timers.hpp"

bool apic_wait_interrupt_handled[MAX_CORE_COUNT] = {false};

namespace apic_timer
{

namespace handlers
{

}

} // namespace apic_timer
