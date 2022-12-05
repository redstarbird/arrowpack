// This file contains functions for gernerating warning messages and error messages
#include "ProblemHandler.h"

void ThrowFatalError(const char *message)
{
    ColorRed();
    printf("Fatal Error: %s\n", message);
    ColorReset();
    exit(1);
}

void CreateWarning(const char *message)
{
    ColorYellow();
    printf("Warning: %s\n", message);
    ColorNormal();
}