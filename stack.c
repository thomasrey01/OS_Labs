#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

void push_stack(uint64_t val, struct Stack *s)
{
    if (s->top == s->size) {
        s->array = (uint64_t *)realloc(s->array, 2 * s->size);
        s->size *= 2;
    }
    s->array[s->top] = val;
    s->top++;
}

struct Stack *new_stack()
{
    struct Stack *s = (struct Stack*)malloc(sizeof(struct Stack));
    s->top = 0;
    s->size = i_size_stack;
    s->array = (uint64_t *)malloc(s->size * sizeof(uint64_t));
    return s;
}

uint64_t pop_stack(struct Stack *s)
{
    if (s->top == 0) {
        printf("Stack Underflow!\n");
        exit(EXIT_SUCCESS);
    }
    s->top--;
    return (s->array[s->top]);
}

uint64_t peek_stack(struct Stack *s)
{
    if (s->top == 0) {
        printf("Empty stack!\n");
        exit(EXIT_SUCCESS);
    }
    return (s->array[s->top - 1]);
}
