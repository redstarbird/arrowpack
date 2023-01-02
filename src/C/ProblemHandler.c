// This file contains functions for gernerating warning messages and error messages
#include "ProblemHandler.h"

void ThrowFatalError(char *message, ...)
{
    va_list args;
    ColorRed();

    va_start(args, message);
    printf("Fatal Error: ");
    vprintf(message, args);
    ColorReset();

    // use va_arg and va_end to access and end the variable arguments
    va_end(args);

    exit(1);
}

void CreateWarning(char *message, ...)
{
    char *ErrorMessage = malloc(strlen(message) + 14);

    ColorYellow();
    printf("Warning: %s\n", message);
    ColorNormal();
}