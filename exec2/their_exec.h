#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

/* Efficient min and max operations */
#define MIN(_a, _b)                                                            \
	({                                                                     \
		typeof(_a) __a = (_a);                                         \
		typeof(_b) __b = (_b);                                         \
		__a <= __b ? __a : __b;                                        \
	})
#define MAX(_a, _b)                                                            \
	({                                                                     \
		typeof(_a) __a = (_a);                                         \
		typeof(_b) __b = (_b);                                         \
		__a >= __b ? __a : __b;                                        \
	})

/* Rounding operations (efficient when n is a power of 2) */
// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)                                                        \
	({                                                                     \
		typeof(a) __a = (a);                                           \
		(typeof(a))(__a - __a % (n));                                  \
	})
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)                                                          \
	({                                                                     \
		typeof(a) __n = (n);                                           \
		(typeof(a))(ROUNDDOWN(a + __n - 1, __n));                      \
	})
