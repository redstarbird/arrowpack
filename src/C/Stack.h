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
    STACK_INT,   // Used for representing a int data type in a stack
    STACK_CHAR,  // Used for representing a char data type in a stack
    STACK_VERTEX // Used for representing a vertex (struct Node) data type in a stack
};

void StackpushV(struct Stack *stack, struct Node *Value);
void StackpushI(struct Stack *stack, int Value);
void StackpopV(struct Stack *stack);
int StackpopI(struct Stack *stack);
bool StackIsEmpty(struct Stack *stack);
bool StackIsFull(struct Stack *stack);
struct Stack *CreateStack(unsigned int Capacity, unsigned int type);

void *Stackpop(struct Stack *stack); // Gets the top value from stack and removes it from stack

#define Stackpush(stack, Value) _Generic(Value, struct Node * \
                                         : StackpushV, int    \
                                         : StackpushI)(stack, Value) // Allows overloading of Stackpush function

#endif // !STACK_H