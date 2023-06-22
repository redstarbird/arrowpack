/* This file is for handling shifts in strings when saved indexes of strings are incorrect because a substring has been inserted or removed which shifts the entire string */
#include "StringShiftHandler.h"

int GetShiftedAmount(int Location, struct ShiftLocation *ShiftLocations) // Returns the new shifted equivalent of a location
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

int GetInverseShiftedAmount(int Location, struct ShiftLocation *ShiftLocations) // Returns the original of a shifted location
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

// Adds a new shift location to a shift locations list
void AddShiftNum(int Location, int ShiftNum, struct ShiftLocation **ShiftLocations, int *ShiftLocationLength)
{
    (*ShiftLocationLength)++;
    *ShiftLocations = realloc(*ShiftLocations, ((*ShiftLocationLength)) * sizeof(struct ShiftLocation));
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