#pragma once
#include "static_assert.h"

#define KPRINTF_CHECK_ARG(x) STATIC_ASSERT(sizeof(x) <= 4, "max 32 bit\n")

#define KPRINTF_CHECK_0()

#define KPRINTF_CHECK_1(a) KPRINTF_CHECK_ARG(a)

#define KPRINTF_CHECK_2(a, b)                                                                                                              \
	KPRINTF_CHECK_ARG(a);                                                                                                                  \
	KPRINTF_CHECK_ARG(b)

#define KPRINTF_CHECK_3(a, b, c)                                                                                                           \
	KPRINTF_CHECK_ARG(a);                                                                                                                  \
	KPRINTF_CHECK_ARG(b);                                                                                                                  \
	KPRINTF_CHECK_ARG(c)

#define KPRINTF_CHECK_4(a, b, c, d)                                                                                                        \
	KPRINTF_CHECK_ARG(a);                                                                                                                  \
	KPRINTF_CHECK_ARG(b);                                                                                                                  \
	KPRINTF_CHECK_ARG(c);                                                                                                                  \
	KPRINTF_CHECK_ARG(d)

#define KPRINTF_CHECK_5(a, b, c, d, e)                                                                                                     \
	KPRINTF_CHECK_ARG(a);                                                                                                                  \
	KPRINTF_CHECK_ARG(b);                                                                                                                  \
	KPRINTF_CHECK_ARG(c);                                                                                                                  \
	KPRINTF_CHECK_ARG(d);                                                                                                                  \
	KPRINTF_CHECK_ARG(e)

#define KPRINTF_CHECK_6(a, b, c, d, e, f)                                                                                                  \
	KPRINTF_CHECK_ARG(a);                                                                                                                  \
	KPRINTF_CHECK_ARG(b);                                                                                                                  \
	KPRINTF_CHECK_ARG(c);                                                                                                                  \
	KPRINTF_CHECK_ARG(d);                                                                                                                  \
	KPRINTF_CHECK_ARG(e);                                                                                                                  \
	KPRINTF_CHECK_ARG(f)

#define KPRINTF_CHECK_7(a, b, c, d, e, f, g)                                                                                               \
	KPRINTF_CHECK_ARG(a);                                                                                                                  \
	KPRINTF_CHECK_ARG(b);                                                                                                                  \
	KPRINTF_CHECK_ARG(c);                                                                                                                  \
	KPRINTF_CHECK_ARG(d);                                                                                                                  \
	KPRINTF_CHECK_ARG(e);                                                                                                                  \
	KPRINTF_CHECK_ARG(f);                                                                                                                  \
	KPRINTF_CHECK_ARG(g)

#define KPRINTF_CHECK_8(a, b, c, d, e, f, g, h)                                                                                            \
	KPRINTF_CHECK_ARG(a);                                                                                                                  \
	KPRINTF_CHECK_ARG(b);                                                                                                                  \
	KPRINTF_CHECK_ARG(c);                                                                                                                  \
	KPRINTF_CHECK_ARG(d);                                                                                                                  \
	KPRINTF_CHECK_ARG(e);                                                                                                                  \
	KPRINTF_CHECK_ARG(f);                                                                                                                  \
	KPRINTF_CHECK_ARG(g);                                                                                                                  \
	KPRINTF_CHECK_ARG(h)

#define KPRINTF_CHECK_9(...) typedef char _kprintf_too_many_args[-1]
