#ifndef STACK_H
#define STACK_H

#include <stdint.h>

const int i_size_stack = 10000;

struct Stack {
    int size;
    int top;
    uint64_t *array;
};

void push_stack(uint64_t val, struct Stack *s);

uint64_t pop_stack(struct Stack *s);

struct Stack *new_stack();

uint64_t peek_stack(struct Stack *s);

void free_stack(struct Stack *s);

#endif
