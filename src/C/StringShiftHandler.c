#include "StringShiftHandler.h"

int GetShiftedAmount(int Location, struct ShiftLocation *ShiftLocations)
{
    unsigned int i = 0;
    unsigned int ShiftNum = 0;
    while (ShiftLocations[i].location != -1)
    {
        if (ShiftLocations[i].location <= Location)
        {
            ShiftNum += ShiftLocations[i].ShiftNum;
            i++;
        }
        else
        {
            break;
        }
    }
    return ShiftNum + Location;
}

int GetInverseShiftedAmount(int Location, struct ShiftLocation *ShiftLocations)
{
    unsigned int i = 0;
    unsigned int ShiftNum = 0;
    while (ShiftLocations[i].location != -1 && ShiftLocations[i].location <= Location)
    {
        if (ShiftNum + ShiftLocations[i].ShiftNum > Location)
        {
            break;
        }
        else
        {
            ShiftNum += ShiftLocations[i].ShiftNum;
        }
        i++;
    }
    return Location - ShiftNum;
}

void AddShiftNum(int Location, int ShiftNum, struct ShiftLocation **ShiftLocations, int *ShiftLocationLength)
{
    (*ShiftLocationLength)++;
    *ShiftLocations = realloc(*ShiftLocations, ((*ShiftLocationLength)) * sizeof(struct ShiftLocation));
    // memcpy(NewShiftLocations, *ShiftLocations, ((*ShiftLocationLength) - 2) * sizeof(struct ShiftLocation));
    // free(*ShiftLocations);
    // *ShiftLocations = NewShiftLocations;
    unsigned int i = 0;
    while (1)
    {
        if ((*ShiftLocations)[i].location >= Location) // Find where to place element so that list is ordered (should probably change to binary search)
        {
            for (int v = (*ShiftLocationLength) - 1; v > i; v--) // Shifts all elements to the right
            {
                (*ShiftLocations)[v] = (*ShiftLocations)[v - 1];
            }
            (*ShiftLocations)[i].location = Location;
            (*ShiftLocations)[i].ShiftNum = ShiftNum;
            (*ShiftLocations)[(*ShiftLocationLength) - 1].location = -1;
            break;
        }
        else if ((*ShiftLocations)[i].location == -1)
        {
            (*ShiftLocations)[i].location = Location;
            (*ShiftLocations)[i].ShiftNum = ShiftNum;
            (*ShiftLocations)[i + 1].location = -1;
            break;
        }
        i++;
    }
}