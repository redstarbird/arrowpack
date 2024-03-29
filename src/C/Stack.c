#include "Stack.h"

struct Stack *CreateStack(unsigned int Capacity, unsigned int type, bool Variable) // Creates and initializes a new stack
{
    size_t TypeSize = 0;

    struct Stack *stack = (struct Stack *)malloc(sizeof(struct Stack));
    stack->capacity = Capacity;
    stack->top = -1;
    stack->Variable = Variable;
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

bool StackIsFull(struct Stack *stack) // Returns whether a given stack is full
{
    return stack->top == stack->capacity - 1;
}

bool StackIsEmpty(struct Stack *stack) // Returns whether a given stack is empty
{
    return stack->top == -1;
}
void StackpushV(struct Stack *stack, struct Node *Value) // Push a value onto a stack of vertices
{
    if (StackIsFull(stack))
    {
        if (stack->Variable)
        {
            stack->capacity *= 2;
            stack->array.VertexArray = realloc(stack->array.VertexArray, stack->capacity * sizeof(struct Node *));
        }
        else
        {
            CreateWarning("Tried to push value to stack, but stack is full");
            return;
        }
    }

    stack->array.VertexArray[++stack->top] = Value;
}
void StackpushI(struct Stack *stack, int Value) // Push a value onto a stack of integers
{
    if (StackIsFull(stack))
    {
        if (stack->Variable)
        {
            stack->capacity *= 2;
            stack->array.IntArray = realloc(stack->array.IntArray, stack->capacity * sizeof(int));
        }
        else
        {
            CreateWarning("Tried to push value to stack, but stack is full");
            return;
        }
    }
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
void *Stackpop(struct Stack *stack) // Gets the top value from stack and removes it from stack
{
    if (StackIsEmpty(stack))
    {
        CreateWarning("Tried to pop value from empty stack\n");
        return NULL;
    }
    switch (stack->Type)
    {
    case STACK_VERTEX:
        return (void *)stack->array.VertexArray[stack->top--];
        break;
    case STACK_INT:
        return (void *)stack->array.IntArray[stack->top--];
        break;
    }
    CreateWarning("Invalid stack type %s\n", stack->Type);
    return NULL;
}