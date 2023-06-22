/* This file contains basic integer functions */
#include "IntFunctions.h"

int min(int x, int y) // Returns the minimum of two integers
{
    return y ^ ((x ^ y) & -(x < y));
}

int max(int x, int y) // Returns the maximum of two integers
{
    return x ^ ((x ^ y) & -(x < y));
}