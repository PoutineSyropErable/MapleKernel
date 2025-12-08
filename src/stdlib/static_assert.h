#pragma once

#ifdef __cplusplus
#    define STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
#    define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif
