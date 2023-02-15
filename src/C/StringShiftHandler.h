#ifndef STRINGSHIFTHANDLER_H
#define STRINGSHIFTHANDLER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct ShiftLocation
{
    int location, ShiftNum;
} ShiftLocation;

int GetShiftedAmount(int Location, struct ShiftLocation *ShiftLocations);

int GetInverseShiftedAmount(int Location, struct ShiftLocation *ShiftLocations);

void AddShiftNum(int Location, int ShiftNum, struct ShiftLocation **ShiftLocations, int *ShiftLocationLength);

#endif // !STRINGSHIFTHANDLER_H