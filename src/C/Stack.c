#include "Stack.h"

struct Stack *CreateStack(unsigned int Capacity, unsigned int type)
{
    size_t TypeSize = 0;

    struct Stack *stack = (struct Stack *)malloc(sizeof(struct Stack));
    stack->capacity = Capacity;
    stack->top = -1;
    switch (type)
    {
    case STACK_INT:
        TypeSize = sizeof(int);
        stack->array.IntArray = malloc(stack->capacity * TypeSize);
        break;
    case STACK_VERTEX:
        TypeSize = sizeof(struct Node *);
        stack->array.VertexArray = malloc(stack->capacity * TypeSize);
        break;
    }
    stack->Type = type;
    return stack;
}

bool StackIsFull(struct Stack *stack)
{
    return stack->top == stack->capacity - 1;
}

bool StackIsEmpty(struct Stack *stack)
{
    return stack->top == -1;
}
void StackpushV(struct Stack *stack, struct Node *Value)
{
    if (StackIsFull(stack))
        return;
    stack->array.VertexArray[++stack->top] = Value;
}
void StackpushI(struct Stack *stack, int Value)
{
    if (StackIsFull(stack))
        return;
    stack->array.IntArray[++stack->top] = Value;
}
int StackpopI(struct Stack *stack) // Need to make overload for this function
{
    if (StackIsEmpty(stack))
    {
        printf("Tried to remove top value from stack but stack is empty");
    }

    return stack->array.IntArray[stack->top--];
}
void StackpopV(struct Stack *stack) // Need to make overload for this function
{
    if (StackIsEmpty(stack))
    {
        printf("Tried to remove top value from stack but stack is empty");
    }
    stack->top--;
    return;
}