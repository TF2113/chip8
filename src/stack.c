#include "stack.h"
#include <stdbool.h>
#include <stdio.h>

void create_stack(Stack *stack) {
    stack->top = -1;
}

bool isEmpty(Stack *stack) {
    return stack->top == -1;
}

bool isFull(Stack *stack) {
    return stack->top >= (16 - 1);
}

void push(Stack *stack, unsigned short new_value) {
    if(isFull(stack)) {
        printf("Stack is full\n");
        return;
    }

    stack->element[++stack->top] = new_value;
}

bool pop(Stack *stack, unsigned short *popped_value){
    if(isEmpty(stack)){
        printf("Stack is empty\n");
        return false;
    }

    *popped_value = stack->element[stack->top];
    --stack->top;
    return true;
}
