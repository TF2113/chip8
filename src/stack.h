#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

typedef struct Stack {
    unsigned short element[16];
    short top;
} Stack;

void create_stack(Stack *stack);
void push(Stack *stack, unsigned short value);
bool pop(Stack *stack, unsigned short *popped_value);

#endif