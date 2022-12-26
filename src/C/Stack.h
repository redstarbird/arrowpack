#ifndef STACK_H
#define STACK_H
#include <stdio.h>
#include <stdbool.h>
#include "../DependencyGraph/DependencyGraph.h"
// A structure to represent a stack
struct Stack
{
    unsigned int Type;
    int top;
    unsigned capacity;
    union
    {
        int *IntArray;
        struct Node **VertexArray;
    } array;
};

enum
{
    STACK_INT,
    STACK_CHAR,
    STACK_VERTEX
};

void StackpushV(struct Stack *stack, struct Node *Value);
void StackpushI(struct Stack *stack, int Value);
void StackpopV(struct Stack *stack);
int StackpopI(struct Stack *stack);
bool StackIsEmpty(struct Stack *stack);
bool StackIsFull(struct Stack *stack);
struct Stack *CreateStack(unsigned int Capacity, unsigned int type);

#define Stackpush(stack, Value) _Generic(Value, struct Node * \
                                         : StackpushV, int    \
                                         : StackpushI)(stack, Value) // Allows overloading of Stackpush function
/*#define Stackpop(stack) _Generic(stack.array, struct Node * * \
                                 : StackpopV, int *           \
                                 : StackpopI)(stack) // Allows overloading of Stackpush function*/
#endif // !STACK_H