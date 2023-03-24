#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

/* Efficient min and max operations */
#define MIN(_a, _b)                                                            \
({                                                                             \
    typeof(_a) __a = (_a);                                                     \
    typeof(_b) __b = (_b);                                                     \
    __a <= __b ? __a : __b;                                                    \
})
#define MAX(_a, _b)                                                            \
({                                                                             \
    typeof(_a) __a = (_a);                                                     \
    typeof(_b) __b = (_b);                                                     \
    __a >= __b ? __a : __b;                                                    \
})

/* Rounding operations (efficient when n is a power of 2) */
// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)                                                        \
({                                                                             \
    typeof(a) __a = (a);                                                       \
    (typeof(a)) (__a - __a % (n));                                             \
})
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)                                                          \
({                                                                             \
    typeof(a) __n = (n);                                                       \
    (typeof(a)) (ROUNDDOWN(a + __n - 1, __n));                                 \
})

/* Scaffolding code to guide the jump to the other binary */
// Finalize is called when the "child" exits
static void finalize() {
    fflush(stdout);
	printf("Finishing up...\n");
}

// This function is used to perform the jump to the loaded binary
// It takes two pointers: the entry point of the new binary and a new stack
// pointer.
_Noreturn void jump(void *entry_point, void *stack_pointer) {
    // We have to do this in pure assembly; this should never return!
    // x86-64 ABI $3.4.1 states the following about registers for new processes:
    //// RBP is unspecified - so we can leave it
    //// RSP should hold the stack pointer
    //// RDX may hold a function pointer to a function that is called after exit,
    ////     or it may hold NULL. We choose the former, to verify proper finish.
    asm volatile (
        "mov %[fin], %%rdx" "\n" // set the finalization function
        "mov %[sp], %%rsp" "\n"  // set the stack pointer
        "jmp *%[ep]"             // jump to the entry point

        // C machinery to pass C variables into assembly
        :: [sp] "r" (stack_pointer), [ep] "r" (entry_point), [fin] "r" (finalize)
        : "rdx" // we exclude RDX from being used by the compiler for the code above
    );

    // We should never get here!
    printf("Error: jump failed or something strange happened\n");
    exit(1);
}
