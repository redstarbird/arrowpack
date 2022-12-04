#include "IntFunctions.h"

int min(int x, int y)
{
    return y ^ ((x ^ y) & -(x < y));
}

int max(int x, int y)
{
    return x ^ ((x ^ y) & -(x < y));
}